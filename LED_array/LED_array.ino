#include "logging.h"
#include "arrayControl.h"
#include "experimentParameters.h"

unsigned long currBlockEnds;
int currBlock = -1;

// --- Block Logic ---
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
    for(led = 0; led < numLEDs; led++){
        currFreq[led] = exp_stimFrequency[led][currBlock];
        currPulseWidth[led] = exp_pulseWidth[led][currBlock];
        updateTriggerDelay(led);
    }
}

void updateBurstParamsForEachLED() {
    for(led = 0; led < numLEDs; led++){
        currBurstFreq[led] = exp_burstFreq[led][currBlock];
        currBurstDuration[led] = exp_burstDuration[led][currBlock];
        updateBurstInterim(led);
    }
}

// Command Logic
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
      default:
        errOut("Unrecognized command!");
  }
}

void setup(){
  // Assign Pins
  for(int i = 0; i < numLEDs; i++) {
    gatePins[i] = i + firstPin;
  }

  // Initialize Pins
  for(int pin = 0; pin < numLEDs; pin++) {
    pinMode(gatePins[pin], OUTPUT);
  }
  
  // Convert block durations to milliseconds
  for(int block = 0; block < numBlocks; block++) {
    blockDuration[block] = blockDuration[block]*1000;
  }

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
