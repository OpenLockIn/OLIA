import sys
from PyQt5 import QtCore, QtGui, QtWidgets, Qt
from PyQt5 import uic
import pyqtgraph as pg
import communicator as com
import numpy as np
import csv

class MainWindow(QtWidgets.QMainWindow):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)    
        uic.loadUi("interface.ui", self)    
        self.measureButton.clicked.connect(self.measureButtonClicked)
        self.freqInput.returnPressed.connect(self.freqButtonClicked)
        self.connectButton.clicked.connect(self.checkPort)
        self.defaultButton.clicked.connect(self.setDefault)
        self.dialogueBox.setText("Connect to a port to start measuring")
        self.filterCheckBox.clicked.connect(self.filterToggled)
        self.refResetButton.clicked.connect(self.recalcExtRef)
        self.outputScalingEdit.returnPressed.connect(self.outputScalingChanged)
        self.harmonicEdit.returnPressed.connect(self.harmonicChanged)
        self.refCheckBox.clicked.connect(self.extRefToggled)
        self.filterOn = False;
        self.thePen = pg.mkPen(color='k', style=QtCore.Qt.SolidLine, width = 2)
        self.axisPen = pg.mkPen(color='k', style=QtCore.Qt.SolidLine, width = 2)
        self.label_style = {'font-size': '12pt'}
        self.gainBox.activated.connect(self.gainChanged)
        self.timeConstantBox.activated.connect(self.timeConstantChanged)
        self.graphWidget.setXRange(-10, 0, padding=0.03)
        self.graphWidget.getAxis('left').setTextPen('k')
        self.graphWidget.getAxis('bottom').setTextPen('k')
        self.graphWidget.getAxis('left').setPen(self.axisPen)
        self.graphWidget.getAxis('bottom').setPen(self.axisPen)
        self.graphWidget.setLabel('left', "R (mV)", **self.label_style)
        self.graphWidget.setLabel('bottom', "t (s)", **self.label_style)
        self.graphWidget.getAxis('bottom').showLabel(show = True)
        self.graphWidget.getAxis('right').setStyle(tickLength = 0)
        self.graphWidget.getAxis('top').setStyle(tickLength = 0)
        self.graphWidget.showAxis('top')
        self.graphWidget.showAxis('right')
        self.graphWidget.getAxis('top').setPen(self.axisPen)
        self.graphWidget.getAxis('right').setPen(self.axisPen)
        self.graphWidget.getAxis('right').setTextPen(Qt.QColor(0,0,0,0))
        self.graphWidget.getAxis('top').setTextPen(Qt.QColor(0,0,0,0))
        self.graphWidget2.setXRange(-10, 0, padding=0.03)
        self.graphWidget2.getAxis('left').setTextPen('k')
        self.graphWidget2.getAxis('bottom').setTextPen('k')
        self.graphWidget2.getAxis('left').setPen(self.axisPen)
        self.graphWidget2.getAxis('bottom').setPen(self.axisPen)
        self.graphWidget2.setLabel('left', "φ (°)", **self.label_style)
        self.graphWidget2.setLabel('bottom', "t (s)", **self.label_style)
        self.graphWidget2.getAxis('bottom').showLabel(show = True)
        self.graphWidget2.getAxis('right').setStyle(tickLength = 0)
        self.graphWidget2.getAxis('top').setStyle(tickLength = 0)
        self.graphWidget2.showAxis('top')
        self.graphWidget2.showAxis('right')
        self.graphWidget2.getAxis('top').setPen(self.axisPen)
        self.graphWidget2.getAxis('right').setPen(self.axisPen)
        self.graphWidget2.getAxis('right').setTextPen(Qt.QColor(0,0,0,0))
        self.graphWidget2.getAxis('top').setTextPen(Qt.QColor(0,0,0,0))
        self.xvals = []
        self.toPlotR = []
        self.toPlotP = []
        self.toPlotX = []
        self.toPlotY = []
        self.dataCurve1 = self.graphWidget.plot(self.xvals, self.toPlotR, pen = self.thePen)
        self.dataCurve2 = self.graphWidget2.plot(self.xvals, self.toPlotP, pen = self.thePen)
        try:
            conf = open("defaults.cfg", "r")
            self.portInput.setText(conf.read())
            conf.close()
            
        except:
            #print("No default port set")
            self.dialogueBox.setText("No default port set")
                
        

        
    def measureButtonClicked(self):
        try:
            if self.measureButton.isChecked() == 0:
                self.measureButton.setText("Measure")
                self.dialogueBox.setText("Measurement stopped.")
                self.recordBox.setDisabled(False)
    
            else:
                filename = self.filenameInput.text()
                if self.recordBox.isChecked():
                    f = open(filename, 'w', newline='')
                    writer = csv.writer(f)
                    writer.writerow(["R (a.u.)","Phase (deg)","Uncorrected phase (deg)","Noise (a.u.)","X (a.u.)","Y (a.u.)","Sin Harmonic n","Sin Harmonic n+1","Sin Harmonic n+2","Cos Harmonic n","Cos Harmonic n+1","Cos Harmonic n+2", "Higher harmonic number n"])
                    
                self.recordBox.setDisabled(True)
                
                self.toPlotR = np.array([])
                self.toPlotP = np.array([])
                self.toPlotX = np.array([])
                self.toPlotY = np.array([])
                self.xvals = []
                
    
                self.measureButton.setText("Stop") #changes the button. this is the go bit
                self.dialogueBox.setText("Running")
                
                while self.measureButton.isChecked() == 1:
                    try:
                        self.phi0 = float(self.phaseInput.text())*(np.pi/180) #get phase offset
                    except:
                        self.phi0 = 0;
                            
                    measurements = com.ReadDevice(self.port).split()
                    clipping = int(measurements[0])
                    if clipping == 1:
                        self.dialogueBox.setText("Clipping!")
                        self.dialogueBox.setStyleSheet("color: red;")
                    elif clipping == 2:
                        self.dialogueBox.setText("Lock failure!")
                        self.dialogueBox.setStyleSheet("color: red;")
                    elif clipping == 3:
                        self.dialogueBox.setText("Lock failure, and clipping!")
                        self.dialogueBox.setStyleSheet("color: red;")
                        
                    else:
                        self.dialogueBox.setText("Running")
                        self.dialogueBox.setStyleSheet("color: black;")
                        
                    rOut = float(measurements[10])
                    

                    xOut = float(measurements[13])
                    yOut = float(measurements[14])
                    phiOut = float(measurements[11]) - self.phi0  #apply phase offset
                    
                    if self.phaseWrapBox.isChecked():    #wrap phase if required                  
                        if phiOut > 0:
                            phiOut = phiOut
                        else:
                            phiOut = phiOut + np.pi
                        
                    phiOut = round(phiOut*(180/np.pi), 5) #convert to degrees and round   
                    
                    self.phaseOutput.setText(str(phiOut))
                    self.xOutput.setText(str(xOut))
                    self.yOutput.setText(str(yOut))
                    
                    
                    self.rOutput.setText(str(measurements[10]))                
                    self.noiseOutput.setText(str(measurements[12]))
                    self.freqOutput.setText(str(measurements[7]))
                    
                    self.toPlotR = np.append(self.toPlotR, rOut) #numpy array of things to plot
                    self.toPlotP = np.append(self.toPlotP, phiOut)
                    self.toPlotX = np.append(self.toPlotX, xOut) #numpy array of things to plot
                    self.toPlotY = np.append(self.toPlotY, yOut)
                    self.plotDomain = int(self.secondsBox.value()*10)

                    self.graphWidget.setXRange(-1*self.secondsBox.value(), 0, padding=0.03)
                    self.graphWidget2.setXRange(-1*self.secondsBox.value(), 0, padding=0.03)
                    
                    if self.toPlotR.size > self.plotDomain: #remove 1st data point if it's too long
                        self.toPlotR = np.delete(self.toPlotR,0)
                        self.toPlotP = np.delete(self.toPlotP,0)
                        self.toPlotX = np.delete(self.toPlotX,0)
                        self.toPlotY = np.delete(self.toPlotY,0)
    
                        
                    
                    if self.recordBox.isChecked():
                        #writer.writerow([str(measurements[10]),str(measurements[11]),str(measurements[12]),str(measurements[13]),str(measurements[14]),str(measurements[15]),str(measurements[16]),str(measurements[17]),str(measurements[18]),str(measurements[19]),str(measurements[20]),str(measurements[21])])
                        writer.writerow([str(rOut),str(phiOut),str(measurements[11]),str(measurements[12]),
                                         str(xOut),str(yOut),str(measurements[15]),str(measurements[16]),
                                         str(measurements[17]),str(measurements[18]),str(measurements[19]),str(measurements[20]),str(measurements[21])])
                    if self.plotBox.isChecked():
                        plotLength = self.toPlotR.size
                        self.xvals = np.linspace(-0.1*(plotLength-1), 0, plotLength)
                        
                        if self.comboBox.currentIndex() == 0:
                            self.graphWidget.setLabel('left', "R (mV)", **self.label_style)
                            self.graphWidget2.setLabel('left', "φ (°)", **self.label_style)
                            self.dataCurve1.setData(self.xvals, self.toPlotR)
                            self.dataCurve2.setData(self.xvals, self.toPlotP)
                        else:
                            self.graphWidget.setLabel('left', "X (mV)", **self.label_style)
                            self.graphWidget2.setLabel('left', "Y (mV)", **self.label_style)
                            self.dataCurve1.setData(self.xvals, self.toPlotX)
                            self.dataCurve2.setData(self.xvals, self.toPlotY)
    
                        
                    app.processEvents()
                if self.recordBox.isChecked():    
                    f.close()
                    
        except AttributeError:
            self.dialogueBox.setText("Measurement failed! Have you connected?")
            self.measureButton.setText("Start")
            self.recordBox.setDisabled(False)
            self.measureButton.setChecked(False)
            
    
    def filterToggled(self):
        com.sendCommand(self.port, 't')
        checking = com.ReadDevice(self.port).split()
        if checking[3] == '0':
            self.dialogueBox.setText("Synchronous filter off")
            self.filterCheckBox.setChecked(False)
            
        elif checking[3] == '1':
            self.dialogueBox.setText("Synchronous filter on")
            self.filterCheckBox.setChecked(True)
        
        app.processEvents()
        
    def gainChanged(self):
        inGain = self.gainBox.currentText() #obtain gain from drop menu
        gainString = 'g' + inGain #gain commands are prefixed by a 'g'
        com.sendCommand(self.port, gainString) #send command to change input gain
        self.dialogueBox.setText("Input gain changed")
        app.processEvents()
        
    def timeConstantChanged(self):
        inTC = self.timeConstantBox.currentText() #obtain gain from drop menu
        TCString = 'e' + inTC #time constant commands prefixed with an 'e'
        com.sendCommand(self.port, TCString) #send command to change time constant
        self.dialogueBox.setText("Time constant changed")
        app.processEvents()
        
        
    def freqButtonClicked(self):
        if self.refCheckBox.isChecked() == 0:            
            setFreq = self.freqInput.text()
            com.sendCommand(self.port, setFreq) #just send the value to change frequency, no prefix
            self.dialogueBox.setText("Frequency updated")
            app.processEvents()
        

    
    def checkPort(self):
        self.port = str(self.portInput.text())
        check = com.isOpen(self.port)    #port checking function in communicator
        
        if check=="ready":
            self.dialogueBox.setText("Ready!")
            checking = com.ReadDevice(self.port).split()
            if checking[3]=='0':                        #check if sync filter is already on and check box if so
                self.filterCheckBox.setChecked(False)
            elif checking[3]=='1':
                self.filterCheckBox.setChecked(True)


                
            if checking[4]=='0':                        #check if external reference is already on and check box and grey out freq input if so
                self.refCheckBox.setChecked(False)
                self.freqInput.setDisabled(False)
            else:
                self.refCheckBox.setChecked(True)
                self.freqInput.setDisabled(True)
            
            preAmpGain = checking[2]
            index = self.gainBox.findText(preAmpGain, QtCore.Qt.MatchFixedString)
            if index>=0:
                self.gainBox.setCurrentIndex(index)
                
            #timeConstantFloat = float(checking[8])
            timeConstant = '{:g}'.format(float('{:.1g}'.format(float(checking[8])))) #format to 1 sig fig
            timeConstantString = str(timeConstant)
            index = self.timeConstantBox.findText(timeConstantString, QtCore.Qt.MatchFixedString) #find index of this time constant within list
            if index>=0:
                self.timeConstantBox.setCurrentIndex(index)
                               
            outputScaling = checking[1]
            self.outputScalingEdit.setText(outputScaling)
            
            
        elif check=="not_ready":
            self.dialogueBox.setText("Not ready.")
        else:
            self.dialogueBox.setText("Not connected!")
        
    def recalcExtRef(self):
        if self.refCheckBox.isChecked():
            com.sendCommand(self.port, 'c') #recalculate ext ref frequency
            self.dialogueBox.setText("Recalculating")
        else:
            self.dialogueBox.setText("Using internal reference") 
                
        app.processEvents()
        
    def extRefToggled(self):
        com.sendCommand(self.port, 'r')
        self.dialogueBox.setText("Toggled external reference")
        if self.refCheckBox.isChecked():
            self.freqInput.setDisabled(True)
        else:
            self.freqInput.setDisabled(False)
                
        app.processEvents()
        
    def outputScalingChanged(self):
        outputScaling = self.outputScalingEdit.text()
        com.sendCommand(self.port, 's' + outputScaling)
        app.processEvents()
        
    def harmonicChanged(self):
        harmonic = self.harmonicEdit.text()
        com.sendCommand(self.port, 'h' + harmonic)
        app.processEvents()
    
    def setDefault(self):
        p = open('defaults.cfg', 'w')
        p.write(self.portInput.text())
        p.close()
        self.dialogueBox.setText("Default port set to " + self.portInput.text())


try:
    pg.setConfigOption('background', Qt.QColor(0,0,0,0))
    pg.setConfigOption('foreground', 'k')
    pg.setConfigOptions(antialias=True)

    app = QtWidgets.QApplication(sys.argv)
    window = MainWindow()
    window.show()
    app.exec_()
except AttributeError:
    pass
