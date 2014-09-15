#include "LED_array.h"
// --- Global Variables -- Do not modify!! ---///

// Array containing pin numbers
int gatePins[numLEDs];

// Booleans, describing state
boolean activeExperiment = false;
boolean manualMode = false;
boolean ledOn[numLEDs], isBursting[numLEDs];

// Event times, for scheduling events
unsigned long nextLEDEvent[numLEDs], nextBurstEvent[numLEDs];
unsigned long currBlockEnds;

// Arrays for storing current parameter values for each LED
float currFreq[numLEDs];
float currBurstFreq[numLEDs];
unsigned long currPulseWidth[numLEDs];
unsigned long currTriggerDelay[numLEDs];
unsigned long currBurstDuration[numLEDs];
unsigned long currBurstInterim[numLEDs];
unsigned int currStimPower[numLEDs];
unsigned int currPwmDutyCycle[numLEDs];

// counter, for blocks
int currBlock = -1;


// --- Block Logic ---
void runStimulation() {
  logEvent("Starting Stimulation");
  document("start", -1);
  manualMode = false;
  activeExperiment = true;
  startBlock(0);
}

void stopStimulation() {
  activeExperiment = false;
  manualMode = false;
  for(int led = 0; led < numLEDs; led++){
    isBursting[led] = false;
  }
  turnAllLEDsOff();
  currBlock = -1;
  document("stop", -1);
  logEvent("Stopped Stimulation");
}

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

void startBlock(int block) {
    logEvent(String("Starting Block ") + (block + 1));
    // set current block to specified block
    currBlock = block;
    // update stimulation parameters for this block
    // currFreq must be updated before pulseWidth
    updateStimParamsForEachLED();
    // update the burst parameters for each LED
    updateBurstParamsForEachLED();
    // Schedule the ending for this block
    currBlockEnds = millis() + blockDuration[currBlock];
    // Start block by turning off all pins and scheduling next event
    // Blocks will always start on a trigger delay
    for(int led = 0; led < numLEDs; led++) {
        startBursting(led);
    }
}

void updateStimParamsForEachLED() {
    for(int led = 0; led < numLEDs; led++){
        currFreq[led] = exp_pulseFreq[led][currBlock];
        currPulseWidth[led] = exp_pulseWidth[led][currBlock]*1000;  // convert ms to us
        updateTriggerDelay(led);
        currStimPower[led] = exp_stimPower[led][currBlock];
        currPwmDutyCycle[led] = 255 - (currStimPower[led]/100.0)*255;
    }
}

void updateBurstParamsForEachLED() {
    for(int led = 0; led < numLEDs; led++){
        currBurstFreq[led] = exp_burstFreq[led][currBlock];
        currBurstDuration[led] = exp_burstDuration[led][currBlock];
        updateBurstInterim(led);
    }
}

// Resets the led's triggerDelay based on its
// currPulseWidth and currFreq
void updateTriggerDelay(int led) {
    currTriggerDelay[led] = calculateTriggerDelay(currFreq[led], currPulseWidth[led]);
}

// returns the appropriate trigger delay (in us) based on the given freq and pw
unsigned long calculateTriggerDelay(float freq, unsigned long pw) {
    // Hz to ms
    unsigned long tFreq = (1/freq)*1000;
    // ms to us
    unsigned long period = tFreq*1000;
    // Calculate triggerDelay
    unsigned long triggerDelay = period - pw;
    return triggerDelay;
}

// Command parsing and execution functions
// read all available chars to a buffer and interpret any complete lines
void readAndInterpretAvailableChars() {
  String serialLine;
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
  // declared here for use with the pw (pulse width) command
  switch (command[0]) {
      // 'S' means start
      case 'S':
        runStimulation();
        break;
      // 'X' means stop
      case 'X':
        stopStimulation();
        break;
      // 'O' means on
      case 'O':
        turnAllLEDsOn();
        break;
      case 'F':
        turnAllLEDsOff();
        break;
      default:
        errOut("Unrecognized command!");
  }
}


void setup(){
  // Assign Pins
  //for(int i = 0; i < numLEDs; i++) {
  //  gatePins[i] = i + firstPin;
  //}

  // All the pwm pins
  gatePins[0] = 4;
  gatePins[1] = 5;
  gatePins[2] = 9;
  gatePins[3] = 10;
  gatePins[4] = 12;
  gatePins[5] = 14;

  // Initialize Pins
  for(int pin = 0; pin < numLEDs; pin++) {
    pinMode(gatePins[pin], OUTPUT);
  }

  turnAllLEDsOff();

  Serial.begin(9600);
}


void loop() {
  // Set up something to include start conditions.  Button Press?
  readAndInterpretAvailableChars();
  
  // Update block when scheduled
  if (activeExperiment && millis() >= currBlockEnds) {
    startNextBlock();
  }

  // Check LEDs
  checkForBurstEvents();
  checkForLEDEvents();
}