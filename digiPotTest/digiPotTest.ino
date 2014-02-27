// Test program for controlling the MCP41010 Digital Potentiometer with the Teensy Microcontroller

// Use the SPI library
#include <SPI.h>

// Slave Select pin is 10 on the Teensy
// The rest of the pins are configured automatically
const int slaveSelectPin = 10;

// Analog Input Pin
const int input = 14;

// Define some Command Bytes
const byte writeBoth = B00010011;
const byte shutDownBoth = B00100011;

// Some more variables
int resValue = 0;

// ---- Functions ---- //
void debugOut(String msg) {
  Serial.print("[TIME]: ");
  Serial.print(millis());
  Serial.print(" [DEBUG]: ");
  Serial.println(msg);
}

void updateResistorValue() {
  setNextResValue();
  writeValueToResistor();
}

void setNextResValue() {
  if (resValue < 256) {
    resValue++;
  } else {
    resValue = 0;
  }
  debugOut(resValue);
}

void writeValueToResistor() {
  // Set Slave to LOW
  digitalWrite(slaveSelectPin, LOW);
  // Send Command Byte "writeBoth"
  SPI.transfer(writeBoth);
  // Send Value Byte
  SPI.transfer(resValue);
  // Set Slave to HIGH
  digitalWrite(slaveSelectPin, HIGH);
}

void readInputPin() {
  float val = analogRead(input);
  float volts = bitsToVolts(val);
  Serial.println(String("Read ") + volts + String("V from analog input"));
}

float bitsToVolts(float bits) {
  float volts = (bits/1023)*3.3;
  return volts;
}
  
void setup() {
  // Set SS as an output
  pinMode(slaveSelectPin, OUTPUT);
  // Input as input
  pinMode(input, INPUT);
  // Initiate SPI
  SPI.begin();
  // Initiate Serial
  Serial.begin(9600);
}

void loop() {
  
  // Set New Pin Value
  updateResistorValue();
  
  // Report Input Pin Value
  readInputPin();
  
  delay(100);
}
