
# This is the I/O display and manager for Matt Friese's ChR2 stimulator system
# Sam Rendall

from twisted.protocols.basic import LineReciever
from twisted.internet.serialport import SerialPort
from twisted.internet.protocol import ClientFactory
from twisted.internet import stdio
import os
import subprocess as sp

# The port on ubuntu.  We'll change this later
teensyPort = "/dev/ACM0"

# The Root object, he ties it all together
class Root(object):

    def __init__(self):
        self.teensyProtocol = TeensyProtocol(self)
        self.ioProtocol = IoCommandProtocol(self)


class TeensyProtocol(LineReceiver):

    def __init__(self, root):
        self.log = openLogFile()
        self.root = root

    def lineReceived(self, line):
        self.log.write(line)
        self.root.ioProtocol.sendLine(line)


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
        if line == "start" or line == "stop":
            self.sendCommandToTeensy(line):
        else:
            self.sendLine("Invalid command.")
            self.transport.write(">>> ")

    def sendCommandToTeensy(self, command):
        self.root.teensyProtocol.runCommand(command)


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