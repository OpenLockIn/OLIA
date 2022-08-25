#include <IntervalTimer.h>
#include <math.h>
#include <ADC.h>
#include <ADC_Module.h>
#include <elapsedMillis.h>
#define pi 3.1415926535897932384626433832795 
#define inMultiplier 64 //ratio of PLL frequency to external reference frequency
#define nHarmonics 3 //number of higher harmonics to calculate (at least 1, max 3). Can be increased if digitisation rate is decreased accordingly

//NOTE: Select clock speed of 816 MHz for correct operation

/////////////////////////////////////DEFAULTS///////////////////////////////////////                                   

#define ADCAv 1 //ADC averages per sample. increasing can reduce noise at the cost of frequency response 

double Fsig =  1000; //default modulation frequency (Hz) for internal reference

double Fsample = 200000; //ADC digitisation rate (Hz) (changes if using external ref or sync filter).

bool useSyncFilter = false; //use synchronous filter only for very low frequencies

double phaseDiff = 0; //Choose phase offset (radians)

unsigned int outputTime = 100; //number of milliseconds between outputs

double timeConstant = 0.6; //time constant for exponential filters

int preAmpGain = 1; //default input gain. 0, 1, 2, 4, 8, 16, 32 and 64 allowed

double outputScale = 10; //scale factor for "analogue" output

double outputScaleCalibration = 0.3149; //calibration factor for output scale (determined experimentally)

double calibrationFactor = 3.3/2.047;

///////////////////////////////////////////////////////////////////////////////////

IntervalTimer lockInTimer;
elapsedMillis timeElapsed;
ADC *adc = new ADC();

int extMode = 0; //0 to internal reference (default), 1 for external reference
volatile int edgeCount = 0;
int isAttached = 0;
int underSampling = 1;
int overSampling = 1;
int timeToSample = 0;

int firstHigherHarmonic = 2;
int lastHigherHarmonic = firstHigherHarmonic + nHarmonics - 1;

int refOutPin = 23; //output reference signal pin
int pwmPin = 22;    //"analogue" output pin (uses filtered pwm)

//pins for PGA preamp
int ampPin0 = 11;
int ampPin1 = 10;
int ampPin2 = 9;


double outVal = 0;

//variables for detecting clipping/saturation
float clip = 0;
float clipAlpha = 0.0005;
volatile float clipValue = 0;
bool clipping;

float lock = 0;
float lockAlpha = 0.0005;
volatile float offLockValue = 0;
bool offLock;

int errorValue; //0 if fine, 1 if clipping, 2 if poor reference lock, 3 if both


int i;
volatile int j;
int k;
double wsig = 2*pi*Fsig; // signal angular frequency
int dt_micros = (int)(1000000/Fsample); //sample time in microseconds
int samplesPerPeriod = (int)(Fsample/Fsig);
double oldFsample = Fsample;
double FsampleTrue = (1.0/((double)dt_micros))*1000000; //after rounding errors
double FsigTrue = FsampleTrue/((double)samplesPerPeriod); //after rounding errors
int sampleIndex = 0; //what sample within the current period are we at
bool wait = true;
double phasSig; //reference signal values in phase
double quadSig; //ditto, quadrature
volatile double delta; //filtering things
volatile double incr;
volatile double xiFilt1[100];
volatile double xqFilt1[100];
volatile double xiFilt2[100];
volatile double xqFilt2[100];
volatile double xiVar2[100];
volatile double xiMean2[100];
volatile double xqVar2[100];
volatile double xqMean2[100];
double grabQuad[100];
double grabPhas[100];
double grabVar;
double lag;
double rmsMeasured;
double noise;
double xSig; //input signal
double xi0;
double xq0;
double xio0;
double xqo0;

volatile unsigned long lastLoopStart;
volatile unsigned long thisLoopStart;
volatile unsigned long thisloopTime;
volatile unsigned long lastLoopTime;
volatile float loopRatio;

double alpha = 1/(FsampleTrue*timeConstant); // filter parameter
double alpha_min = 1.0 - alpha;

//sync filter circular buffer and indices
int xBuffer[50000] = {};
int Io0 = 1;
int In0 = 0;


void setup() {
  pinMode(12, INPUT_PULLDOWN); //external reference trigger from PLL circuit
  pinMode(refOutPin, OUTPUT);
  pinMode(pwmPin, OUTPUT);

  pinMode(ampPin0, OUTPUT);
  pinMode(ampPin1, OUTPUT);
  pinMode(ampPin2, OUTPUT);
  
  analogWriteResolution(10); //set PWM output resolution and frequency
  analogWriteFrequency(22, 146484.38);// fastest possible for 10 bit output
  Serial.begin(2000000);

  setGain(); //set preamp gain to default value

  //adc configuration
  delay(1000);
  adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::HIGH_SPEED); 
  adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::HIGH_SPEED);
  adc->adc0->setResolution(12);
  adc->adc0->setAveraging(ADCAv);
  adc->adc0->startContinuous(A0);
  lockInTimer.begin(calculate, dt_micros);
  delay(1000);
}

void loop() {
  while(wait == true){
    if (timeElapsed > outputTime){
      timeElapsed = 0;
      wait = false;   
    } 
  }

  //serial interface
  if (Serial.available() > 0){
      // ignore line feed and carriage return
      if (Serial.peek() == '\n' || Serial.peek() == '\r')
      {
         Serial.read();  // read and discard LF & CR
      }
      else if (Serial.peek() == 't'){ //toggle synchronous filter
        Serial.read();
        if (useSyncFilter == true){         
          if(extMode==0){ //if using internal reference            
            lockInTimer.end();
          }
          else{ //if using external reference
            detachInterrupt(digitalPinToInterrupt(12)); 
          }          
          delay(100);
          useSyncFilter = false;
          Fsample = oldFsample; //return to OG sample frequency
          setNewFreqs();
          softerReboot();
          clearBuffers(); //reset buffers
          if(extMode==0){ //if using internal reference            
            lockInTimer.begin(calculate, dt_micros);//if using internal reference!!
          }
          else{ //if using external reference
            if (overSampling == 1){
              attachInterrupt(digitalPinToInterrupt(12), calculate, RISING);
            }
            else{
              attachInterrupt(digitalPinToInterrupt(12), calculate, CHANGE);
            }     
          }                     
          delay(100);  
        }
        else if (useSyncFilter == false){
          if(extMode==0){ //if using internal reference            
            lockInTimer.end();
          }
          else{ //if using external reference
            detachInterrupt(digitalPinToInterrupt(12)); 
          }  
          delay(100);
          useSyncFilter = true;
          oldFsample = Fsample;
          Fsample = 50000; //needs lower sample rate because memory
          setNewFreqs();
          softerReboot();  
          clearBuffers(); //reset buffers        
          if(extMode==0){ //if using internal reference            
            lockInTimer.begin(calculate, dt_micros);//if using internal reference!!
          }
          else{ //if using external reference
            if (overSampling == 1){
              attachInterrupt(digitalPinToInterrupt(12), calculate, RISING);
            }
            else{
              attachInterrupt(digitalPinToInterrupt(12), calculate, CHANGE);
            } 
          }   
          delay(100);              
        }
      }
      else if (Serial.peek() == 'r'){ //toggle reference mode
        Serial.read();
        switchReferenceMode();
        softerReboot();
      }
      else if (Serial.peek() == 'c'){ //recalculate in frequency
        Serial.read();
        recalculateAlpha();
        softerReboot();
      }
      else if (Serial.peek() == 'g'){ //set new amp gain
        preAmpGain = Serial.parseInt(); //ignores g character
        setGain();
      }
      else if (Serial.peek() == 'e'){ //set new exponential filter time const
        timeConstant = Serial.parseFloat(); //ignores e character
        alpha = 1/(FsampleTrue*timeConstant); // how unusual
        alpha_min = 1.0 - alpha;
      }
      else if (Serial.peek() == 's'){ //set new output scale factor
        outputScale = Serial.parseFloat(); //ignores s character         
      }
      else if (Serial.peek() == 'h'){ //set first higher harmonic
        firstHigherHarmonic = Serial.parseInt(); //ignores h character  
        lastHigherHarmonic = firstHigherHarmonic + nHarmonics - 1;     
      }
      //change frequency (internal reference only)
      else
      { 
         lockInTimer.end();
         delay(100);
         Fsig = Serial.parseFloat();
         setNewFreqs();
         softerReboot();
         clearBuffers();
         lockInTimer.begin(calculate, dt_micros);
         delay(100);
      }
   }
  //turn off interrupts to grab values 
  noInterrupts();
  //grab fundamentals
  grabPhas[0] = xiFilt2[0]*calibrationFactor;
  grabQuad[0] = xqFilt2[0]*calibrationFactor;

  //grab harmonics
  for (i = firstHigherHarmonic - 1; i < lastHigherHarmonic; ++i){
    grabPhas[i] = xiFilt2[i]*calibrationFactor;
    grabQuad[i] = xqFilt2[i]*calibrationFactor;
  }
  
  grabVar = xqVar2[0];
  interrupts(); //back on again

  //calculate phase and R
  lag = atan(grabQuad[0]/grabPhas[0]);
  rmsMeasured = sqrt(grabQuad[0]*grabQuad[0] + grabPhas[0]*grabPhas[0]);
  noise = sqrt(grabVar)*calibrationFactor;

  //heuristic value to determine whether clipping is happening
  if (clipValue > 0.05){
    clipping = true;
  }
  else{
    clipping = false;
  }
  //likewise for failure to lock to external ref
  if (offLockValue > 0.2){
    offLock = true;
  }
  else{
    offLock = false;
  }

  if (clipping == true && offLock == true){
    errorValue = 3;
  }
  else if (clipping == true){
    errorValue = 1;
  }
  else if (offLock == true){
    errorValue = 2;
  }
  else{
    errorValue = 0;
  }
  
 
  Serial.print(errorValue); 
  Serial.print(" ");
  Serial.print(outputScale); //for analogue out
  Serial.print(" ");
  Serial.print(preAmpGain);
  Serial.print(" ");
  
  Serial.print(useSyncFilter); //1 if yes, 0 no
  Serial.print(" ");
  Serial.print(extMode); //0 for internal, 1 for external
  Serial.print(" ");
  Serial.print(samplesPerPeriod);
  Serial.print(" ");
  Serial.print(FsampleTrue); //current sample rate
  Serial.print(" ");

  Serial.print(FsigTrue); //current signal frequency under measurement
  Serial.print(" ");
  Serial.print(timeConstant);
  Serial.print(" ");

  if (extMode == 0){
    Serial.print(0);
    Serial.print(" ");
  }
  else if (overSampling == 2){
    Serial.print(0.5);
    Serial.print(" ");
  }
  else {
    Serial.print(underSampling);
    Serial.print(" ");
  }
  
  Serial.print(rmsMeasured, 5); //total signal amplitude
  Serial.print(" "); 
  Serial.print(lag, 5); //phase measurement
  Serial.print(" ");
  Serial.print(noise, 5);  //noise estimate
  Serial.print(" ");
  Serial.print(grabPhas[0], 5);  //fundamental in-phase
  Serial.print(" ");
  Serial.print(grabQuad[0], 5);  //fundamental quadrature
  Serial.print(" ");



  //output harmonics
    for (i = firstHigherHarmonic - 1; i < lastHigherHarmonic; ++i){
      Serial.print(grabPhas[i], 5);  //in phase 
      Serial.print(" ");
    }
    for (i = firstHigherHarmonic - 1; i < lastHigherHarmonic; ++i){
      Serial.print(grabQuad[i], 5);  //quad
      Serial.print(" ");
    }

  Serial.print(firstHigherHarmonic);  //first higher harmonic
  //Serial.print(" ");


  
  Serial.println(""); //new line at end of output string

  //outVal = outputScale*rmsMeasured;
  outVal = outputScale*rmsMeasured*outputScaleCalibration;
  analogWrite(pwmPin, outVal); //output recovered amplitude via filtered pwm

  wait = true;
  
}

void calculate(){ //function for actually doing the lock-in
  ++timeToSample;
  if (timeToSample == underSampling){ //if undersampling > 1 then doesn't run every trigger
       
    xSig = (double)adc->analogReadContinuous(A0); //sample current adc value

    if (xSig < 96|| xSig > 4000){ //clipping if outside these bounds (it's a (nominally) 12 bit ADC)
      clip = 1;
    }
    else{
      clip = 0;
    }
        
    clipValue = clipValue*(1 - clipAlpha) + clip*clipAlpha; //filter clippingness

    thisLoopStart = micros();
    thisloopTime = thisLoopStart - lastLoopStart;
    
    loopRatio = (float)thisloopTime/(float)lastLoopTime;

    lastLoopTime = thisloopTime;
    lastLoopStart = thisLoopStart;

    if (loopRatio < 0.95 || loopRatio > 1.05){
      lock = 1;
    }
    else {
      lock = 0;
    }
    
    offLockValue = offLockValue*(1 - lockAlpha) + lock*lockAlpha; //filter poor lockingness
    
    // if using standard exponential filter (not synchronous)
    if (useSyncFilter == false){
  
      //in-phase harmonics
      for (j = 0; j < lastHigherHarmonic; ++j){
        //calculate reference value and multiply (-1* because preamp is inverting)
        phasSig = -1*sin(((2.0 * (j + 1) * pi * sampleIndex) / samplesPerPeriod) - ((j + 1) * phaseDiff));
        xi0 = (xSig*phasSig); //-1 because preamp is inverting
  
        //first smoothing
        delta = xi0 - xiFilt1[j]; 
        incr = alpha*delta;
        xiFilt1[j] = xiFilt1[j] + incr;
        
        //second smoothing
        delta = xiFilt1[j] - xiFilt2[j]; 
        incr = alpha*delta;
        xiFilt2[j] = xiFilt2[j] + incr;

        if (j == 0){ //skip to first higher harmonic
          j = firstHigherHarmonic - 2;          
        }
      }
      //same for quadrature harmonics
      for (j = 0; j < lastHigherHarmonic; ++j){
        
        //calculate reference value and multiply
        quadSig = -1*cos(((2.0 * (j + 1) * pi * sampleIndex) / samplesPerPeriod) - ((j + 1) * phaseDiff)); 
        xq0 = (xSig*quadSig);
  
        //first smoothing
        delta = xq0 - xqFilt1[j];
        incr = alpha*delta;
        xqFilt1[j] = xqFilt1[j] + incr;
        
        //second smoothing
        delta = xqFilt1[j] - xqFilt2[j];
        incr = alpha*delta;
        xqFilt2[j] = xqFilt2[j] + incr; 
        
        if (j == 0){ //skip to first higher harmonic
          j = firstHigherHarmonic - 2;          
        }
      }
      
      //noise estimate with quadrature harmonic
      delta = xqFilt2[0] - xqMean2[0];
      incr = alpha*delta;
      xqMean2[0] = xqMean2[0] + incr;
      xqVar2[0] = alpha_min*(xqVar2[0] + delta*incr);
      
      ++sampleIndex;
    }
  
    
    //if using optional synchronous filter instead (only for very low frequencies)
    if (useSyncFilter == true){
      for (j = 0; j < lastHigherHarmonic; ++j){
        phasSig = -1*sin(((2.0 * (j + 1) * pi * sampleIndex) / samplesPerPeriod) - ((j + 1) * phaseDiff));
        xi0 = (xSig*phasSig); //multiplication by reference signal
        xio0 = (xBuffer[Io0]*phasSig); //ditto for oldest buffered value
        
        //smooth
        xiFilt1[j] = xiFilt1[j] + ((xi0 - xio0)/samplesPerPeriod);
        delta = xiFilt1[j] - xiFilt2[j];
        incr = alpha*delta;
        xiFilt2[j] = xiFilt2[j] + incr;

        if (j == 0){ //skip to first higher harmonic
          j = firstHigherHarmonic - 2;          
        }
        
      }
      for (j = 0; j < lastHigherHarmonic; ++j){
        quadSig = -1*cos(((2.0 * (j + 1) * pi * sampleIndex) / samplesPerPeriod) - ((j + 1) * phaseDiff)); 
        xq0 = (xSig*quadSig); 
        xqo0 = (xBuffer[Io0]*quadSig);
        
        xqFilt1[j] = xqFilt1[j] + ((xq0 - xqo0)/samplesPerPeriod);      
        delta = xqFilt1[j] - xqFilt2[j];
        incr = alpha*delta;
        xqFilt2[j] = xqFilt2[j] + incr;
  
        //noise calculation
        delta = xqFilt2[j] - xqMean2[j];
        incr = alpha*delta;
        xqMean2[j] = xqMean2[j] + incr;
        xqVar2[j] = (1 - alpha) * (xqVar2[j] + delta*incr);  

        if (j == 0){ //skip to first higher harmonic
          j = firstHigherHarmonic - 2;          
        }
      }
  
      //advance circular buffers
      In0 = In0 + 1;
      if (In0 >= samplesPerPeriod){
        In0 = In0 - samplesPerPeriod;
      }
      Io0 = In0 + 1;
      if (Io0 >= samplesPerPeriod){
        Io0 = Io0 - samplesPerPeriod;
      }
      //store newest value in buffer
      xBuffer[In0] = xSig;
      
      ++sampleIndex; 
    } 
    //output reference signal
    if (sampleIndex < (samplesPerPeriod/2)){
      digitalWrite(refOutPin, HIGH);
    }
    else {
      digitalWrite(refOutPin, LOW);
    }    
    if (sampleIndex >= samplesPerPeriod){
      sampleIndex = 0;
    }
    timeToSample = 0;
  }
}

void softReboot() { //more of a hard reboot actually
  SCB_AIRCR = 0x05FA0004;
}

void softerReboot(){ //reinitialise arrays and buffer indices
  Io0 = 1;
  In0 = 0;
  sampleIndex = 0;
  for (j = 0; j < 100; ++j){
    xqFilt1[j] = 0;
    xqFilt2[j] = 0; 
  }
  for (j = 0; j < 100; ++j){
    xiFilt1[j] = 0;
    xiFilt2[j] = 0; 
  } 
}

void setNewFreqs(){ //recalculate parameters based on new frequencies
  samplesPerPeriod = (int)(Fsample/Fsig);
  dt_micros = (int)(1000000/Fsample); //sample time in microseconds
  FsampleTrue = (1.0/((double)dt_micros))*1000000;
  FsigTrue = FsampleTrue/((double)samplesPerPeriod);
  alpha = 1/(FsampleTrue*timeConstant); // how unusual
  alpha_min = 1.0 - alpha;
}

void switchReferenceMode(){
  //code that changes whether internal or external reference is used 
  if(extMode==0){//if not using external reference, start    
    lockInTimer.end();//stop loop defined by internal sample rate
    recalculateAlpha(); //work out sample and signal frequencies and calculate filter coeff
    //"calculate" fires whenever detecting a rising edge. (or changing edge for slow freqs)   
    extMode = 1;
  }
    
  else{//if using external reference, stop    
    detachInterrupt(digitalPinToInterrupt(12)); //stop calculate happening on trigger pin
    isAttached = 0; //detached for next time    
    if(useSyncFilter == true){ //set default (sample) frequencies again
      Fsample = 50000;
    }
    else{
      Fsample = 200000;
    }   
    setNewFreqs(); //set freqs and recalculate filter coeff.
    lockInTimer.begin(calculate, dt_micros); //revert back to internal ref mode
    extMode = 0;
    underSampling = 1;
    timeToSample = 0;
  }
}

void recalculateAlpha(){ //calculate parameters for external reference mode
  
  if(isAttached == 1){//stop interrupts so i can count edges
    detachInterrupt(digitalPinToInterrupt(12));
  }
  
  //code that measures the external reference frequency and uses it to calculate alpha
  attachInterrupt(digitalPinToInterrupt(12), countEdge, CHANGE); //count changing edges 5s
  edgeCount = 0;
  delay(5000); //wait 5s for it to count up
  noInterrupts();
  Fsample = (double)edgeCount/(2*5); //grab the count
  interrupts();
  Fsig = (double)Fsample/(inMultiplier);
  detachInterrupt(digitalPinToInterrupt(12));//finished so disconnect
  

  if(Fsample <= 100000){
    underSampling = 1;  //sample twice per loop
    overSampling = 2;
    samplesPerPeriod = inMultiplier*2; //factor of 2 because sampling twice as fast
    attachInterrupt(digitalPinToInterrupt(12), calculate, CHANGE);
  }
  else if(Fsample <= 200000){
    underSampling = 1;  //sample every loop
    overSampling = 1;
    samplesPerPeriod = inMultiplier;
    attachInterrupt(digitalPinToInterrupt(12), calculate, RISING);
  }
  if (Fsample > 200000){
    underSampling = 2; //sample every second loop
    overSampling = 1;
    samplesPerPeriod = inMultiplier/2;
    attachInterrupt(digitalPinToInterrupt(12), calculate, RISING);
  }
  if (Fsample > 400000){
    underSampling = 4; //sample every 4th loop
    overSampling = 1;
    samplesPerPeriod = inMultiplier/4;
    attachInterrupt(digitalPinToInterrupt(12), calculate, RISING);
  }
  if (Fsample> 800000){
    underSampling = 8; //sample every 8th loop
    overSampling = 1;
    samplesPerPeriod = inMultiplier/8;
    attachInterrupt(digitalPinToInterrupt(12), calculate, RISING);
  }
  isAttached = 1;
  FsigTrue = Fsig;
  FsampleTrue = (overSampling*Fsample)/underSampling;

  alpha = 1/(FsampleTrue*timeConstant); //approximately
  alpha_min = 1.0 - alpha;
  
}

void countEdge(){
  ++edgeCount; //guess what this does
}

void setGain(){
  if(preAmpGain == 0){
    digitalWrite(ampPin0, LOW);
    digitalWrite(ampPin1, LOW);
    digitalWrite(ampPin2, LOW);
    calibrationFactor = (3.3/2.047); 
  }
  if(preAmpGain == 1){
    digitalWrite(ampPin0, HIGH);
    digitalWrite(ampPin1, LOW);
    digitalWrite(ampPin2, LOW);
    calibrationFactor = (3.3/2.047)/preAmpGain; 
  }
  if(preAmpGain == 2){
    digitalWrite(ampPin0, LOW);
    digitalWrite(ampPin1, HIGH);
    digitalWrite(ampPin2, LOW);
    calibrationFactor = (3.3/2.047)/preAmpGain;  
  }
  if(preAmpGain == 4){
    digitalWrite(ampPin0, HIGH);
    digitalWrite(ampPin1, HIGH);
    digitalWrite(ampPin2, LOW);
    calibrationFactor = (3.3/2.047)/preAmpGain;  
  }
  if(preAmpGain == 8){
    digitalWrite(ampPin0, LOW);
    digitalWrite(ampPin1, LOW);
    digitalWrite(ampPin2, HIGH);
    calibrationFactor = (3.3/2.047)/preAmpGain;  
  }
  if(preAmpGain == 16){
    digitalWrite(ampPin0, HIGH);
    digitalWrite(ampPin1, LOW);
    digitalWrite(ampPin2, HIGH);
    calibrationFactor = (3.3/2.047)/preAmpGain;  
  }
  if(preAmpGain == 32){
    digitalWrite(ampPin0, LOW);
    digitalWrite(ampPin1, HIGH);
    digitalWrite(ampPin2, HIGH);
    calibrationFactor = (3.3/2.047)/preAmpGain;  
  }
  if(preAmpGain == 64){
    digitalWrite(ampPin0, HIGH);
    digitalWrite(ampPin1, HIGH);
    digitalWrite(ampPin2, HIGH);
    calibrationFactor = (3.3/2.047)/preAmpGain;  
  }
}

void clearBuffers(){ //clears out buffer for synchronous filter
  for (k = 0; k < 50000; ++k){
    xBuffer[k] = 0;
  }
  Io0 = 1;
  In0 = 0;  
}

/* pin combos for preamp gain
 * gain 0 1 2
 * 
 * 0    0 0 0
 * 1    1 0 0
 * 2    0 1 0
 * 4    1 1 0
 * 8    0 0 1
 * 16   1 0 1
 * 32   0 1 1
 * 64   1 1 1 
 *  
 */

//ayy lmao
