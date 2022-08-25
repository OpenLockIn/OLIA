# Usage instructions - quick start guide

## Connections and power

The signal input is connected across the input terminal and any ground (marked "In" and "GND"). OLIA accepts input in the range -1.6 to +1.6 V relative to the common ground. If an external reference signal is being used, it should be connected to the terminal marked "Ref" on the input block. OLIA accepts a 3.3 V TTL square wave as an external reference.

The terminal marked "Out" provides an analogue output (discussed below), and "ADC" is connected to the input signal as it is fed into the teensy's ADC (post analogue signal conditioning). A 3.3 V TTL square wave signal is output at the "Ref" terminal (on the output block) that is in phase and of equal frequency to the internal reference. Also provided is a 5 V terminal, which passes through power from  the USB connection to allow powering of external hardware (such as the included optical DC servo board), and an analogue output ("Out"). The voltage at the analogue output is proportional to the current value of R (the lock-in magnitude) multiplied by a scaling factor (default 10), and is obtained from a low-pass filtered PWM signal. 

Once all external connections are established, OLIA can be powered on by connecting the USB cable.  



## Software
OLIA is included with a simple python 3 frontend (OLIAGUI.py). The software requires the libraries PyQt5, numpy, pyqtgraph and pyserial to run. The first two of these libraries are included by default with [Anaconda](https://www.anaconda.com/), and the latter two can be installed using the included conda package manager. To do this, first install Anaconda, then run `conda install -c anaconda pyserial` in the Anaconda prompt to install pyserial (following the prompts as they arise). After installation has finished, run `conda install -c anaconda pyqtgraph` to install pyqtgraph. 

The script "communicator.py" and ui file "interface.ui" must be included in the same directory as OLIAGUI.py. A screenshot of the control software is included below. **OLIA can also be used without the packaged frontend, or you may wish to build your own; to allow this an explanation of the serial interface is included [here.](https://github.com/ajharvie/OLIA/blob/main/doc/SerialRef.md)**

<img src="https://github.com/ajharvie/OLIA/blob/main/doc/images/frontend.png" width=50% height=50%>

## Connecting to OLIA
OLIA communicates to the computer using USB serial (the USB connection also provides power). Once connected, type the correct serial address into the "Port" box (COM7 in the screenshot) and click "connect". If desired, the current port can be set as the default for future connections by hitting the "Set default" button.

## Running a measurement
### Starting
To start a measurement, connect OLIA as described above and hit the button marked "Start". Selecting the "Save to file" checkbox before starting a measurement will result in the data being recorded to a .csv-format file in the working directory. The filename can be specified in the text box. If the magnitude of the input signal is near the -1.6 or +1.6 V limit, the software will report "clipping"; the used must then reduce the magnitude of the input signal or reduce the input gain. 

### Specifying frequencies and filters
By default, OLIA powers on in internal reference mode at a default frequency of 1 kHz. To change the frequency, type the desired frequency in Hz into the box and hit enter. The synchronous filter can be activated or deactivated using a checkbox. If the synchronous filter is not being used, the filter time constant can be set using the drop-down box, as can the input gain (default 1, max 64).  All of the above configuration settings can be changed "on the fly" during a measurement.

### Outputs
The current values of the recovered lock-in amplitude *R* and phase *ϕ*, a measurement of the current noise, as well as the in-phase and quadrature amplitudes *X* and *Y* will be displayed in their respective boxes and updated at a rate of 10 Hz. The values of **either** *R* and *ϕ* **or** *X* and *Y* are plotted in real time on the axes as shown in the screenshot. The time domain of the plot can be changed using the drop down box. Selecting the checkbox labelled "Wrap phase" will cause the output value of the phase to be wrapped to the interval 0 - 2π.  If information on higher harmonics is required, the first higher harmonic can be specified in the box. The lock-in magnitude at this harmonic number, as well as that for the next two harmonics will be recorded to the file (for example, if the "first higher harmonic" is set to 5, the 5th, 6th and 7th harmonics will be recorded - both in-phase and quadrature - as well as the fundamental). The analogue output provides a voltage signal proportional to the current lock-in amplitude *R* multiplied by a scaling factor (default 10); this can be changed at any time using the text entry box.

### Using the external reference
The external reference mode can be activated using a checkbox (note that the external reference signal must be connected before powering on the device). On activating external reference mode, OLIA will pause output for around 3 seconds while it initialises. By default, OLIA accepts external square-wave, 3.3 V TTL reference signals of frequencies between approximately 100 - 6000 Hz. If required, this range can be changed by changing the value of C8 (820 pF by default); higher values of C8 will result in lower values for the usable frequency range, and vice-versa. The external reference signal is used to generate a frequency-multiplied "trigger" signal using a phase-locked loop (PLL) in combination with a counter. Correct operation of the PLL requires tuning of the centre frequency of the PLL's internal oscillator to a value of a similar order of magnitude as the "trigger" signal (64 times the input reference frequency). This is achieved by manually adjusting the potentiometer. To find the right setting, connect the external reference signal and switch to external referncing mode. Adjust the potentiometer knob until the front panel no longer shows a lock failure (in big red letters). After recovering from a lock failuire, the user should press "Query freq." to recalculate the incoming reference frequency. For frequencies above ~1 kHz, the potentiometer will usually be turned fully to the left, and only turned to the right for lower frequencies. 

The frequency of the external reference signal can be measured and displayed at any time by clicking the button marked "Query freq.". Note that this temporarily pauses the lock-in calculation while it reinitialises.

## Stopping a measurement
To finish running a measurement, click the "Stop" button. OLIA can then simply be disconnected from power by unplugging the USB cable.
