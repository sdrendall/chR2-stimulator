#include <SPI.h>
// ----- User Defined Experiment Parameters -----//

// The number of blocks in the experiment.
// Each block represents a specific stimulation frequency and pulse width
// Over a specified duration
const int numBlocks = 9;

// Block Parameters.  Each parameter consists of a list of values corresponding to the
// value of that parameter in each respective block.
// THE NUMBER OF ENTRIES IN EACH LIST MUST BE EQUAL TO numBlocks!

// Boolean.  0 indicates no stimulation.
// pulseWidth, stimFrequency and stimPower will be set automatically
// if stimulate[block] == 0 
boolean stimulate[numBlocks] = {1, 1, 1, 1, 1, 1, 1, 1, 1};

// Block durations, in seconds
unsigned long blockDuration[numBlocks] = {5, 5, 5, 5, 5, 5, 5, 5, 5};

// Pulse width in milliseconds
unsigned long pulseWidth[numBlocks] = {250, 250, 250, 250, 0, 250, 250, 250, 250};

// Stimulation Frequency in Hz
float stimFrequency[numBlocks] = {1, 1, 1, 1, 1, 2, 2, 2, 2};

// Stimulation power (percentage)
float stimPower[numBlocks] = {100, 75, 50, 25, 0, 25, 50, 75, 100};


// Pins, specify first and last pin to be used.
const int firstPin = 22;
const int lastPin = 22;



// ----- Program Parameters (do not edit) -----//
// Containers for holding information about blocks
int currBlock = -1;
unsigned long triggerDelay[numBlocks];

// Timers, for scheduling
unsigned long currBlockEnds;
unsigned long nextPinEvent;

// Booleans for decision making
boolean activeExperiment = false;
boolean pinsOn = false;
boolean stimulating = 0;

// Define some Command Bytes
const byte writeBoth = B00010011;
const byte shutDownBoth = B00100011;

// container for resistor value
int resValue = 255;

// container for PWM duty cycle
int maxPower = 255*75/100;
int dutyCycle = 255;
const int gatePin = 22;

// Slave Select pin is 10 on the Teensy
// The rest of the pins are configured automatically
const int slaveSelectPin = 10;

const int numPins = lastPin - firstPin + 1;
int PINS[numPins];

String serialLine;

//For Debugging
float stimPeriod[numBlocks];

// -----     Functions     -----//
// The LED logic is inverted with respect to the teensy's output
// For Matt's convenience, he will input the triggerDelay as pulseWidth
// Here we will calculate the actual pulse width and set triggerDelay using pulseWidth
void calculateTriggerDelay(int block) {
  // Convert Hz to us
  stimPeriod[block] = (1/stimFrequency[block])*1000000;
  // set triggerDelay
  triggerDelay[block] = pulseWidth[block];
  // Calculate pulseWidth
  pulseWidth[block] = stimPeriod[block] - triggerDelay[block];
}

// Logging Functions
void logEvent(String msg) {
  Serial.print("[TIME]: ");
  Serial.print(millis());
  Serial.print(" [LOG]: ");
  Serial.println(msg);
}

void debugOut(String msg) {
  Serial.print("[TIME]: ");
  Serial.print(millis());
  Serial.print(" [DEBUG]: ");
  Serial.println(msg);
}

void errOut (String err) {
  Serial.print("[TIME]: ");
  Serial.print(millis());
  Serial.print(" [ERROR]: ");
  Serial.println(err);
} 

// Pin control functions
void updatePins() {
  if (pinsOn) {
    turnPinsOff();
  } else {
    turnPinsOn();
  }
  scheduleNextPinEvent();
}

void turnPinsOff() {
  for(int i = 0; i < numPins; i++) {
    digitalWrite(PINS[i], LOW);
  }
  pinsOn = false;  
}

void turnPinsOn() {
  for (int i = 0; i < numPins; i++) {
    digitalWrite(PINS[i], HIGH);
  }
  pinsOn = true;
}

// pulseWidth and triggerDelay in us
void scheduleNextPinEvent() {
  if (pinsOn) {
    nextPinEvent = micros() + pulseWidth[currBlock];
  } else {
    nextPinEvent = micros() + triggerDelay[currBlock];
  }
}

// SPI resistor control functions
void updateLedPower() {
  dutyCycle = (stimPower[currBlock]/100)*255;
  setFETDutyCycle(dutyCycle);
}

void setFETDutyCycle(int dc) {
  // Invert duty cycle, transistor logic is 
  // inverse
  dc = 255 - dc;
  debugOut(dc);
  analogWrite(gatePin, dc);
}

void writeValueToResistor(int value) {
  debugOut(String("Setting res Value to ") + value) ;
  // Set Slave to LOW
  digitalWrite(slaveSelectPin, LOW);
  // Send Command Byte "writeBoth"
  SPI.transfer(writeBoth);
  // Send Value Byte
  SPI.transfer(value);
  // Set Slave to HIGH
  digitalWrite(slaveSelectPin, HIGH);
  debugOut("Res Value Set");
}

// State control functions
void runStimulation() {
  logEvent("Starting Stimulation");
  activeExperiment = true;
  startBlock(0);
}

void stopStimulation() {
  activeExperiment = false;
  stimulating = false;
  turnPinsOff();
  currBlock = -1;
  logEvent("Stopped Stimulation");
}  

void startBlock(int block) {
  logEvent(String("Starting Block ") + (block + 1));
  debugOut(String("Trigger Delay: ") + triggerDelay[block]);
  // set current block to specified block
  currBlock = block;
  // update LED power
  updateLedPower();
  // check if I should stimulate
  if (stimulate[currBlock]) {
    stimulating = true;
  } else {
    stimulating = false;
  }
  // Schedule the ending for this block
  currBlockEnds = millis() + blockDuration[currBlock];
  // Start block by turning off all pins and scheduling next event
  // Blocks will always start on a trigger delay
  turnPinsOff();
  scheduleNextPinEvent();
}

// Function to start next block
// Should not be used to start stimulation!
// Call runStimulation() instead
void startNextBlock() {
  currBlock++;
  // If currBlock has reached numBlocks or is 0
  // (-1 after incrementing),
  // we shouldn't be stimulating
  if (currBlock < numBlocks && currBlock > 0) {
    startBlock(currBlock);
  } else {
    stopStimulation();
  } 
}

void startManualMode() {
  if (activeExperiment) {
    stopStimulation();
  }
}

// Command parsing and execution functions
// read all available chars to a buffer and interpret any complete lines
void readAndInterpretAvailableChars() {
    while (Serial.available() > 0){
      // read any available characters
      char ch = Serial.read();
      if (ch == '\n') {
        // read any available characters
        interpretInputString(serialLine);
        serialLine = "";
        break; // don't interpret more than one line before returning
      } else {
      serialLine += ch;
    }
  }
}

void interpretInputString(String input) {
  String command, argin;
  long int arg;
  // Remove leading and trailing whitespace
  input.trim();
  
  if (input.length() == 1) {
    command = input;
    arg = -1;
  } else if (input.indexOf(':') == 1) {
     // parameter value
    int i = input.indexOf(':');
    command = input.substring(0, i);
    argin = input.substring(i+1);
    argin.trim();
    arg = argin.toInt();
  } else {
    errOut(String("Unexpected Input: \"") + input + String("\""));
  }
  executeCommand(command, arg);
}

void executeCommand(String command, long int arg) {
  switch (command[0]) {
      // 'S' means start
      case 'S':
        debugOut("Recieved Start Command");
        runStimulation();
        break;
      // 'X' means stop
      case 'X':
        debugOut("Recieved Stop Command");
        stopStimulation();
        break;
      // 'M' means enter manual mode
      case 'M':
        startManualMode();
        break;
      // 'P' is for POWAH
      case 'P':
        //writeValueToResistor(arg);
        setFETDutyCycle(arg);
        break;
      // 'O' is on
      case 'O':
        turnPinsOn();
        break;
      // 'F' is off
      case 'F':
        turnPinsOff();
        break;        
      case 'R':
        analogWriteFrequency(gatePin, arg);
        break;
      case 'T':
        writeValueToResistor(arg);
        break;
      default:
        errOut("Unrecognized command!");
  }
}

// setup and loop
void setup(){
  // Specify pins as outputs
  for(int i = 0; i < numPins; i++){
    PINS[i] = i + firstPin;
    pinMode(PINS[i], OUTPUT);
  }
  pinMode(gatePin, OUTPUT);
  analogWriteFrequency(gatePin, 200);
  
  // Convert block durations to milliseconds
  for (int i = 0; i < numBlocks; i++) {
    blockDuration[i] = blockDuration[i] * 1000;
  }
  
  // Convert Pulse Width to microseconds
  for (int i = 0; i < numBlocks; i++) {
    pulseWidth[i] = pulseWidth[i]*1000;
  }
  
  // Overwrite block parameters where !stimulate
  for (int i = 0; i < numBlocks; i++){
    if (!stimulate[i]) {
      pulseWidth[i] = 0;
      stimFrequency[i] = 1/blockDuration[i];
      stimPower[i] = 0;
    }
  }
  
  // Calculate Trigger Delays
  for (int i = 0; i < numBlocks; i++){
    calculateTriggerDelay(i);
  }

  // Initialize serial communication
  Serial.begin(9600);
  
  // Setup SPI
  pinMode(slaveSelectPin, OUTPUT);
  SPI.begin();
}
  

void loop() {
  // Set up something to include start conditions.  Button Press?
  readAndInterpretAvailableChars();
  
  // Update block when scheduled
  if (activeExperiment && millis() >= currBlockEnds) {
    startNextBlock();
  }
    
  // Check Pins when scheduled, pin events use micros()
  if (stimulating && (micros() >= nextPinEvent)) {
    updatePins();
  }
}
