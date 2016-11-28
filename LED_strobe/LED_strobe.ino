// ----- User Defined Experiment Parameters -----//

// The number of blocks in the experiment.
// Each block represents a specific stimulation frequency and pulse width
// Over a specified duration
const int numBlocks = 11;

// Block Parameters.  Each parameter consists of a list of values corresponding to the
// value of that parameter in each respective block.
// THE NUMBER OF ENTRIES IN EACH LIST MUST BE EQUAL TO numBlocks!

/*
// FULL POWER CURVE //
// Block durations, in seconds
unsigned long blockDuration[numBlocks] = {15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15};

// Pulse width in milliseconds
float pulseWidth[numBlocks] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

// Stimulation Frequency in Hz
float stimFrequency[numBlocks] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

// Stimulation power (percentage)
float stimPower[numBlocks] = {0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
*/


unsigned long blockDuration[numBlocks] = {60, 60, 60, 60, 60, 60, 60, 60, 60, 60, 60};

// Pulse width in milliseconds
float pulseWidth[numBlocks] = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000};

// Stimulation Frequency in Hz
float stimFrequency[numBlocks] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

// Stimulation power (percentage)
float stimPower[numBlocks] = {0, 2, 4, 6, 8, 10, 15, 20, 40, 80, 100};


// ----- Program Parameters (do not edit) -----//
// Containers for holding information about blocks
int currBlock = -1;
unsigned long triggerDelay[numBlocks];

// Timers, for scheduling
unsigned long currBlockEnds;
unsigned long nextLEDEvent;

// For logging
unsigned long startTime = millis();

// Booleans for decision making
boolean activeExperiment = false;
boolean manualMode = false;
boolean ledOn = true;

// containers for PWM
float maxPower = 255; // Can be used to limit the maximum power output by the device
const int gatePin = 15;

// global variables used as output parameters, set to defaults
float currPower = 50; //% of maxPower
float currFreq = 1;
unsigned long currTriggerDelay = 800;
unsigned long currPulseWidth = 200;

String serialLine;

// -----     Functions     ----- //
void calculateTriggerDelayForEachBlock() {
  for (int i = 0; i < numBlocks; i++){
    triggerDelay[i] = calculateTriggerDelay(stimFrequency[i], pulseWidth[i]);
  }
}

// returns a trigger delay value based on a given frequency and pulse width
unsigned long calculateTriggerDelay(float freq, unsigned long pw) {
  // Convert Hz to us
  // Convert float to long first
  // Hz to ms
  unsigned long tFreq = (1/freq)*1000;
  // ms to us
  unsigned long stimPeriod = tFreq*1000;
  // Calculate triggerDelay
  unsigned long triggerDelay = stimPeriod - pw;
  return triggerDelay;
}

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

void logData(String parameter, float value) {
  if (parameter == "start") {
    startTime = millis();
  }
  Serial.print("data ");
  Serial.print(millis() - startTime);
  Serial.print(String(" ") + parameter + String(" "));
  Serial.println(value);
}

// LED and Pin control functions
void toggleLED() {
  if (activeExperiment || manualMode) {
    scheduleNextLEDEvent();
  }
  if (ledOn) {
    turnLEDOff();
  } else {
    turnLEDOn();
  }
}

void turnLEDOff() {
  // Use a global for current power
  // setLEDToCurrPower();
  digitalWrite(gatePin, LOW);
  logData("led", 0);
  ledOn = false;
}

void turnLEDOn() {
  // Have to do something about setting power here...
  // digitalWrite alone will fail under certain PWM conditions
  pinMode(gatePin, OUTPUT);
  setLEDToCurrPower();
  logData("led", currPower);
  ledOn = true;
}

void updatePulseFrequency(float freq) {
  // Called to set the frequency, uses the current pulse width
  currFreq = freq;
  updatePulseWidth(currPulseWidth/1000); // Function takes pulse width in milliseconds
}

// Calculates the trigger delay based on
// the given pulse width (IN MILLISECONDS!!) and the currFreq
// Converts user entered value from milliseconds to microseconds
void updatePulseWidth(unsigned long pw) {
  currPulseWidth = pw * 1000;
  currTriggerDelay = calculateTriggerDelay(currFreq, currPulseWidth);
}

// pulseWidth and triggerDelay in us
// LED EVENTS ARE SCHEDULED BEFORE LED STATED IS UPDATED!!
void scheduleNextLEDEvent() {
  if (ledOn) {
    // If the light is on, I'm about to turn it off.
    nextLEDEvent = micros() + currTriggerDelay;
  } else {
    // If the light is off, I'm about to turn it on
    nextLEDEvent = micros() + currPulseWidth;
  }
}

// PWM functions
void updateCurrPower(float power) {
  currPower = power;
  // Reset the LED's output if it is already on
  if (ledOn) {
    setLEDToCurrPower();
  }
}

// resets the LED's power to currPower
void setLEDToCurrPower() {
  if (currPower == 0) {
    turnLEDOff();
  } else {
    // LED state is inverse to the pin's state
    // invert power value here
    int dc = (currPower/100)*255;
    // Reset pin to avoid bugs
    pinMode(gatePin, OUTPUT);
    // Set new PWM duty cycle
    analogWrite(gatePin, dc);
  }
}

// State control functions
void runStimulation() {
  logEvent("Starting Stimulation");
  logData("start", -1);
  manualMode = false;
  activeExperiment = true;
  startBlock(0);
}

void stopStimulation() {
  activeExperiment = false;
  manualMode = false;
  turnLEDOff();
  currBlock = -1;
  logData("stop", -1);
  logEvent("Stopped Stimulation");
}

void startBlock(int block) {
  logEvent(String("Starting Block ") + (block + 1));
  // set current block to specified block
  currBlock = block;
  // update stimulation parameters for this block
  // currFreq must be updated before pulseWidth
  currFreq = stimFrequency[currBlock];
  updateCurrPower(stimPower[currBlock]);
  updatePulseWidth(pulseWidth[currBlock]);
  // Schedule the ending for this block
  currBlockEnds = millis() + blockDuration[currBlock];
  // Start block by turning off all pins and scheduling next event
  // Blocks will always start on a trigger delay
  scheduleNextLEDEvent();
  turnLEDOff();
}

// Function to start next block
// Should not be used to start stimulation!
// Call runStimulation() instead
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

void startManualMode() {
  if (activeExperiment) {
    stopStimulation();
  }
  manualMode = true;
}

void startPulsing() {
  logEvent("Starting Manual Pulse");
  logData("start", -1);
  if (! manualMode) {
    startManualMode();
  }
  toggleLED();
}

// Command parsing and execution functions
// read all available chars to a buffer and interpret any complete lines
void readAndInterpretAvailableChars() {
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
  unsigned long pw;
  float freq;
  switch (command[0]) {
      // 'S' means start
      case 'S':
        runStimulation();
        break;
      // 'X' means stop
      case 'X':
        stopStimulation();
        break;
      // 'M' means enter manual mode
      case 'M':
        startManualMode();
        break;
      // 'P' is for POWAH
      case 'P':
        updateCurrPower(arg);
        break;
      // 'O' is on
      case 'O':
        turnLEDOn();
        break;
      // 'F' is off
      case 'F':
        turnLEDOff();
        break;     
      // 'R' changes the pwm frequency  
      case 'R':
        // analogWriteFrequency(gatePin, int(arg)); // Not supported by Teensy 2.0
        break;
      // 'H' changes the pulse frequency
      case 'H':
        // Arg will be sent as mHz, convert back to Hz
        freq = arg;
        freq = freq/1000;
        updatePulseFrequency(freq);
        break;
      // 'W' changes the pulse width
      case 'W':
        pw = arg;
        updatePulseWidth(pw);
        break;
      // 'U' tells the teensy to start pulsing
      case 'U':
        // Stop any ongoing experiments
        if(activeExperiment) {
          logEvent("Warning!  Stopping current experiment and entering manual mode!");
        }
        startManualMode();
        startPulsing();
        break;
      // Return an error if an unexpected command is encountered
      default:
        errOut("Unrecognized command!");
  }
}

// setup and loop
void setup() {
  // Set up PWM
  pinMode(gatePin, OUTPUT);
  // analogWriteFrequency(gatePin, 200); // Not supported by teensy 2.0
  turnLEDOff();
  
  // Convert block durations to milliseconds
  for (int i = 0; i < numBlocks; i++) {
    blockDuration[i] = blockDuration[i] * 1000;
  }

  // Calculate Trigger Delays
    calculateTriggerDelayForEachBlock();

  // Initialize serial communication
  Serial.begin(9600);
}
  

void loop() {
  // Set up something to include start conditions.  Button Press?
  readAndInterpretAvailableChars();
  
  // Update block when scheduled
  if (activeExperiment && millis() >= currBlockEnds) {
    startNextBlock();
  }

  // Toggle LED when necessary, uses us
  if ((activeExperiment || manualMode) && micros() >= nextLEDEvent) {
    toggleLED();
  }
}
