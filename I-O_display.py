
# This is the I/O display and manager for Matt Friese's ChR2 stimulator system
# Sam Rendall

from twisted.protocols.basic import LineReceiver
from twisted.internet.serialport import SerialPort
from twisted.internet.protocol import ClientFactory
from twisted.internet import stdio
import os, sys
import subprocess as sp

# The port on ubuntu.  We'll change this later
teensyPort = "/dev/ttyACM0"

# The Root object, he ties it all together
class Root(object):

    def __init__(self):
        self.ioProtocol = IoCommandProtocol(self)
        self.teensyProtocol = TeensyProtocol(self)


class TeensyProtocol(LineReceiver):

    root = None

    def __init__(self, root):
        self.log = openLogFile()
        self.root = root

    def lineReceived(self, line):
        sys.stdout.flush()
        self.log.write(line + '\n')
        self.root.ioProtocol.sendLine(line)
        self.root.ioProtocol.transport.write('>>> ')


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

    
    def do_power(self, val1, val2=None, dP=1, dt=.5):
        """
        Manual Mode Only!!!

        Set the Power to a specified value (0-255)
        If a second value is specified, queue a sequence of power changes
        by default, power will increment by dP=1 every dt=.5 seconds.  These
        values can be specified as well."""
        if val2:
            powerSequence = range(val1, val2+dP, dP)
            for i, val in enumerate(powerSequence):
                from twisted.intenet import reactor
                reactor.callLater(dt*(i+1), self.do_power, val)
        else:
            self.sendLine("P:" + val1)

    def do_on(self):
        """ Turn on the LED """
        self.sendLine("O")

    def do_off(self):
        """ Turn off the LED """
        self.sendLine("F")




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




def openLogFile():
    log = open('stimLog.txt', 'a')
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