float LowTargetTemp = 34;
float HighTargetTemp = 36;

int fanPin = 11;
int sensorPin;

float checkTemperatureSensor() {
	// Read from sensor

	// Convert to temperature
}

void turnFanOff() {
	//Set pin LOW
}

void turnFanOn() {
	// Make pin HIGH
}

void setup() {

	// Set fan controller pin to OUTPUT

	// Set temp sensor pin to INPUT
}

void loop() {
	// 1. Check temp
	currTemp = checkTemperatureSensor();

	// 2. Update fan
	if (currTemp < LowTargetTemp && fanOn) {
		turnFanOff();
	} else if (currTemp > HighTargetTemp && !fanOn) {
		turnFanOn();
	}
}

/* Functions I would look up 

pinMode
digitalWrite
digitalRead
analogRead
*/
