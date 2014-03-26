#include "arrayControl.h"

// Booleans, describing state
extern boolean activeExperiment;
extern boolean manualMode;
extern boolean ledOn[numLEDs], isBursting[numLEDs];

// Event times, for scheduling events
extern unsigned long nextLEDEvent[numLEDs], nextBurstEvent[numLEDs];

// Arrays for storing current parameter values for each LED
extern float currFreq[numLEDs];
extern float currBurstFreq[numLEDs];
extern unsigned long currPulseWidth[numLEDs];
extern unsigned long currTriggerDelay[numLEDs];
extern unsigned long currBurstDuration[numLEDs];
extern unsigned long currBurstInterim[numLEDs];

extern int gatePins[numLEDs];


// --- LED LOGIC ---
void turnLEDOff(int led) {
    // Incompatible with PWM.  Assumes that LEDs are only being controlled
    // With digitalWrite()
    digitalWrite(gatePins[led], HIGH);
    document("led", 0);
    ledOn[led] = false;
}

void turnLEDOn(int led) {
    // Doesn't involve PWM.  Turns the LED all the way on
    digitalWrite(gatePins[led], LOW);
    document("led", 1);
    ledOn[led] = true;
}

void turnAllLEDsOff() {
    for(int led = 0; led < numLEDs; led++) {
        turnLEDOff(led);
    }
}

void turnAllLEDsOn() {
    for(int led = 0; led < numLEDs; led++) {
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

void toggleLED(int led) {
    if (activeExperiment || manualMode) {
        scheduleNextLEDEvent(led);
    }
    if (ledOn[led]) {
        turnLEDOff(led);
    } else {
        turnLEDOn(led);
    }
}

void checkForLEDEvents() {
    // loop through LEDs
    for(int led = 0; led < numLEDs; led++) {
        // Toggle LED when necessary, uses us
        if ((activeExperiment || manualMode) && micros() >= nextLEDEvent[led]) {
            toggleLED(led);
        }
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


// --- BURST LOGIC ---
void checkForBurstEvents() {
    for (int led = 0; led < numLEDs; led++) {
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
        nextBurstEvent[led] = micros() + currBurstInterim[led];
    } else {
        nextBurstEvent[led] = micros() + currBurstDuration[led];
    }
} 

void startBursting(int led) {
    // Start new LED train
    toggleLED(led);
    isBursting[led] = true;
}

void stopBursting(int led) {
    // override led schedule
    nextLEDEvent[led] = micros() + currBurstInterim[led]*1000;
    turnLEDOff(led);
    isBursting[led] = false;
}

// sets the led's currBurstInterim based on its
// currBurstDuration and currBurstFreq
void updateBurstInterim(int led) {
    unsigned long burstPeriod = (1/currBurstFreq[led])*1000;
    currBurstInterim[led] = burstPeriod - currBurstDuration[led];
}
