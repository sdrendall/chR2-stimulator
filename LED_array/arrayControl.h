#ifndef arrayControl_h
#define arrayControl_h

#include "Arduino.h"
#include "experimentParameters.h"
#include "logging.h"

void turnLEDOff(int led);
void turnLEDOn(int led);
void scheduleNextLEDEvent(int led);
void toggleLED(int led);
void checkForLEDEvents();
void updateTriggerDelay(int led);
void checkForBurstEvents();
void toggleBurstState(int led);
void scheduleNextBurstEvent(int led);
void startBursting(int led);
void stopBursting(int led);
void updateBurstInterim(int led);
void turnAllLEDsOff();
void turnAllLEDsOn();
unsigned long calculateTriggerDelay(float freq, unsigned long pw);

#endif