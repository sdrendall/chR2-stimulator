# This is the I/O display and manager for Matt Friese's ChR2 stimulator system
# Sam Rendall

from twisted.protocols.basic import LineReceiver
from twisted.internet.serialport import SerialPort
from twisted.internet.protocol import ClientFactory
from twisted.internet import stdio
from math import floor
import os, sys, datetime
import subprocess as sp

# The serial port the teensy is connected to
teensyPort = "/dev/tty.usbmodem12341"

# The Root object, he ties it all together
class Root(object):

    def __init__(self):
        self.ioProtocol = IoCommandProtocol(self)
        self.teensyProtocol = TeensyProtocol(self)


class TeensyProtocol(LineReceiver):

    root = None

    def __init__(self, root):
        self.log = openLogFile()
        self.dataFile = None
        self.root = root


    def lineReceived(self, line):
        if self.isData(line):
            # log data to the csv
            self.logData(line)
        else:
            # otherwise write it to the log
            # and print it to stdout
            self.log.write(line + '\n')
            self.root.ioProtocol.sendLine(line)
            self.root.ioProtocol.transport.write('>>> ')

    def logData(self, line):
        timestamp, event, value = line.split()[1:]
        if event == "start":
            if self.dataFile is not None:
                self.closeDataFile()
            self.dataFile = openDataFile()
        elif event == "stop":
            self.closeDataFile()
        elif event == "led":
            self.writeLineToDataFile(timestamp, value)

    def writeLineToDataFile(self, timestamp, value):
        if not self.dataFile:
            self.dataFile = openDataFile()
        dataLine = self.generateDataLine(timestamp, value)
        self.dataFile.write(dataLine)

    def generateDataLine(self, timestamp, value):
        now = datetime.datetime.now()
        line = '{},{},{}'.format(timestamp, value, now)
        return line + os.linesep

    def closeDataFile(self):
        if self.dataFile:
            df, self.dataFile = self.dataFile, None
            df.close()

    def isData(self, line):
        if line.split()[0] == "data":
           return True
        else:
           return False

    ## COMMANDS FOR THE TEENSY!

    def do_start(self):
        """Start Stimulation Paradigm"""
        self.sendLine("S")

    def do_stop(self):
        """Stop Ongoing Stimulation Paradigm"""
        self.sendLine("X")

    def do_manual(self):
        """Tell Teensy to Enter Manual Mode"""
        self.sendLine("M")

    
    def do_power(self, val1, val2=None, dP=None, dt=.5):
        """
        Manual Mode Only!!!

        Set the Power to a specified value (0-100)
        If a second value is specified, queue a sequence of power changes
        by default, power will increment by dP=1 every dt=.5 seconds.  These
        values can be specified as well."""
        if val2:
            val1, val2 = int(val1), int(val2)
            if not dP:
                # Set dP to be ascending or descending based on
                # val1 and val2
                if val2 > val1:
                    dP = 1
                else:
                    dP = -1
            powerSequence = range(val1, val2+dP, dP)
            for i, val in enumerate(powerSequence):
                from twisted.internet import reactor
                reactor.callLater(dt*i, self.do_power, str(val))
        else:
            self.sendLine("P:" + str(val1))

    def do_pwmfreq(self, freq):
        """ Set the teensy's PWM frequency """
        self.sendLine("R:" + freq)

    def do_pulse(self):
        """ Tell the teensy to start pulsing """
        self.sendLine("U")

    def do_freq(self, freq):
        """ Set the pulse frequency """
        # Convert freq to mHz to allow for floating point transfer
        freq = str(float(freq)*1000)
        self.sendLine("H:" + freq)

    def do_pw(self, pw):
        """ Set the pulse width """
        self.sendLine("W:" + pw)

    def do_on(self):
        """ Turn on the LED """
        self.sendLine("O")

    def do_off(self):
        """ Turn off the LED """
        self.sendLine("F")

    def do_pot(self, val1, val2=None, dV=None, dt=.5):
        """ Adjust the potentiometer
            Input is similar to power adjustment """
        if val2:
            val1, val2 = int(val1), int(val2)
            if not dV:
                # Set dV to be ascending or descending based on
                # val1 and val2
                if val2 > val1:
                    dV = 1
                else:
                    dV = -1
            valSequence = range(val1, val2+dV, dV)
            from twisted.internet import reactor
            for i, val in enumerate(valSequence):
                reactor.callLater(dt*i, self.do_pot, str(val))
        else:
            self.sendLine("T:" + str(val1))




class IoCommandProtocol(LineReceiver):
    from os import linesep as delimiter

    def __init__(self, root):
        self.root = root

    def connectionMade(self):
        self.transport.write('>>> ')

    def lineReceived(self, line):
        # Ignore blank lines
        if not line: return
        self.parseLine(line)
        self.transport.write('>>> ')

    def parseLine(self, line):
        # Split it up
        allArgs = line.split()
        command = allArgs[0].lower()
        args = allArgs[1:]
        # Send it off
        self.sendCommandToTeensy(command, args)

    def sendCommandToTeensy(self, command, args):

        try:
            method = getattr(self.root.teensyProtocol, "do_" + command)
        except AttributeError, e:
            self.sendLine("[Error]: Command not recognized")
        else:
            try:
                method(*args)
            except Exception, e:
                self.sendLine("[Error]: " + str(e))


def openDataFile():
    baseName = "dataLog"
    dt = datetime.datetime.now()
    dateString = "{:04}{:02}{:02}_{:02}{:02}".\
        format(dt.year, dt.month, dt.day, dt.hour, dt.minute)
    # open dataFile
    dataFile = open(os.path.join("{}_{}.csv".format(baseName, dateString)), "w")
    return dataFile

def openLogFile():
    log = open('eventLog.txt', 'a')
    currentDateAndTime = sp.check_output("date", shell=True)
    log.write("-" * 15 + "\n")
    log.write(currentDateAndTime + "\n")
    return log


def main():
    from twisted.internet import reactor
    root = Root()

    port = SerialPort(root.teensyProtocol, teensyPort, reactor, baudrate=9600)
    stdio.StandardIO(root.ioProtocol)

    reactor.run()

if __name__ == "__main__":
    main()
