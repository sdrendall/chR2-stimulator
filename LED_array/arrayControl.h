#include experimentParameters.h

// --- LED LOGIC ---
void checkForLEDEvents() {
    // loop through LEDs
    for(led = 0; led < numLEDs; led++) {
        // Toggle LED when necessary, uses us
        if ((activeExperiment || manualMode) && micros() >= nextLEDEvent[led]) {
            toggleLED(led);
        }
    }
}

void toggleLED(int led) {
    if (activeExperiment || manualMode) {
        scheduleNextLEDEvent(led);
    }
    if (ledOn) {
        turnLEDOff(led);
    } else {
        turnLEDOn(led);
    }
}

void scheduleNextLEDEvent(int led) {
  if (ledOn[led]) {
    // If the light is on, I'm about to turn it off.
    nextLEDEvent[led] = micros() + currTriggerDelay[led];
  } else {
    // If the light is off, I'm about to turn it on
    nextLEDEvent[led] = micros() + currPulseWidth[led];
  }
}

void turnLEDOff(int led) {
    // Incompatible with PWM.  Assumes that LEDs are only being controlled
    // With digitalWrite()
    digitalWrite(gatePins[led], HIGH);
    document("led", 0);
    ledOn = false;
}

void turnLEDOn(int led) {
    // Doesn't involve PWM.  Turns the LED all the way on
    digitalWrite(gatePins[led], LOW);
    document("led", 1);
    ledOn = true;
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
}


// --- BURST LOGIC ---
void checkForBurstEvents() {
    for (led = 0; led < numLEDs; led++) {
        if ((activeExperiment || manualMode) && millis() >= nextBurstEvent[led]) {
            toggleBurstState(led);
        }
    }
}

void toggleBurstState(int led) {
    scheduleNextBurstEvent(led);
    if (isBursting[led]) {
        stopBursting(led);
    } else {
        startBursting(led);
    }
}

void scheduleNextBurstEvent(int led) {
    // Schedule then next burst event, dependent on the led's bursting state
    // Designed to be immediately followed by a call to stopBursting() or startBursting()
    if (isBursting[led]) {
        nextBurstEvent[led] = micros() + burstInterim[led];
    } else {
        nextBurstEvent[led] = micros() + burstDuration[led];
    }
} 

void startBursting(int led) {
    // Start new LED train
    toggleLED(led);
}

void stopBursting(int led) {
    // override led schedule
    nextLEDEvent[led] = micros() + burstInterim[led]*1000;
    turnLEDoff(led);
}

// sets the led's currBurstInterim based on its
// currBurstDuration and currBurstFreq
void updateBurstInterim(int led) {
    unsigned long burstPeriod = (1/currBurstFreq[led])*1000;
    currBurstInterim[led] = burstPeriod - currBurstDuration[led];
}


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
    for(led = 0; led < numLEDs; led++) {
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