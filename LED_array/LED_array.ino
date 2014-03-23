#include arrayControl.h

unsigned long currBlockEnds;

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



void setup(){

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