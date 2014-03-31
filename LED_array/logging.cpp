#include "logging.h"

unsigned long startTime;

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

void errOut(String err) {
  Serial.print("[TIME]: ");
  Serial.print(millis());
  Serial.print(" [ERROR]: ");
  Serial.println(err);
} 

void document(String parameter, float value) {
  // Temporarily disabled
  return
  if (parameter == "start") {
    startTime = millis();
  }
  Serial.print("data ");
  Serial.print(millis() - startTime);
  Serial.print(String(" ") + parameter + String(" "));
  Serial.println(value);
}
