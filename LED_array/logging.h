#ifndef logging_h
#define logging_h

#include "Arduino.h"

void logEvent(String msg);
void debugOut(String msg);
void errOut(String err);
void document(String parameter, float value);

#endif
