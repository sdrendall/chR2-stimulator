#ifndef LED_array_h
#define LED_array_h

#include "experimentParameters.h"
#include "arrayControl.h"
#include "logging.h"

void runStimulation();
void stopStimulation();
void startNextBlock();
void startBlock(int block);
void updateStimParamsForEachLED();
void updateBurstParamsForEachLED();
void interpretInputString(String input);
void executeCommand(String command, long int arg);

#endif