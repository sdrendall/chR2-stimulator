
# This is the I/O display and manager for Matt Friese's ChR2 stimulator system
# Sam Rendall

from twisted.protocols.basic import LineReceiver
from twisted.internet.serialport import SerialPort
from twisted.internet.protocol import ClientFactory
from twisted.internet import stdio
import os
import subprocess as sp

# The port on ubuntu.  We'll change this later
teensyPort = "/dev/ttyACM0"

# The Root object, he ties it all together
class Root(object):

    def __init__(self):
        self.ioProtocol = IoCommandProtocol(self)
        self.teensyProtocol = TeensyFactory(self).buildProtocol(None)


class TeensyProtocol(LineReceiver):

    root = None

    def __init__(self):
        self.log = openLogFile()

    def lineReceived(self, line):
        self.log.write(line + '\n')
        self.root.ioProtocol.sendLine(line)

    def do_start(self):
        self.sendLine("S")

    def do_stop(self):
        self.sendLine("X")


# Hacky way to catch connectionFailed failures
class TeensyFactory(ClientFactory):

    protocol = TeensyProtocol

    def __init__(self, root):
        self.root = root

    def clientConnectionFailed(self, reason):
        root.ioProtocol.sendLine("Connection to Teensy Failed!")

    def buildProtocol(self, addr):
        proto = ClientFactory().buildProtocol(addr)
        proto.root = self.root
        return proto

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
        command = lower(allArgs[0])
        args = allArgs[:1]

        # Send it off
        self.sendCommandToTeensy(command, args)

    def sendCommandToTeensy(self, command, args):
        self.root.teensyProtocol.runCommand(command)

        try:
            method = getattr(self, "do_" + command)
        except AttributeError, e:
            self.sendLine("[Error]: Command not recognized")
        else:
            try:
                method()
            except Exception, e:
                self.sendLine("[Error]: " + e)




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