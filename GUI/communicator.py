# -*- coding: utf-8 -*-
"""
Created on Mon Nov 26 12:40:38 2018

@author: andrewjh
"""

import serial

def isOpen(port):
    try:
        erm = serial.Serial(port, timeout=1)
        
        if erm.read():
            return "ready"
            
        else: 
            return "not_ready"
        
        
    except:
        return "not_connected"
        
def ReadDevice(port):
    
    """
    ReadDevice(Port)
    
    Port - String - name of serial port

    """

    ser = serial.Serial(port)
    ser.flushInput()

    
    while True:
        try:
            ser_bytes = ser.readline()
            reading = ser_bytes[0:len(ser_bytes)-2].decode("utf-8")
        
            return reading
                
        except:
            print("No")
            break
        
def sendCommand(port, command):
    ser = serial.Serial(port)
    ser.write(bytes(command, 'utf-8'))