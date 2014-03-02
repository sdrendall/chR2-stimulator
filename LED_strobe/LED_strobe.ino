// ----- User Defined Experiment Parameters -----//

// The number of blocks in the experiment.
// Each block represents a specific stimulation frequency and pulse width
// Over a specified duration
const int numBlocks = 9;

// Block Parameters.  Each parameter consists of a list of values corresponding to the
// value of that parameter in each respective block.
// THE NUMBER OF ENTRIES IN EACH LIST MUST BE EQUAL TO numBlocks!

// Block durations, in seconds
unsigned long blockDuration[numBlocks] = {5, 5, 5, 5, 5, 5, 5, 5, 5};

// Pulse width in milliseconds
unsigned long pulseWidth[numBlocks] = {250, 250, 250, 250, 0, 250, 250, 250, 250};

// Stimulation Frequency in Hz
float stimFrequency[numBlocks] = {1, 1, 1, 1, 1, 2, 2, 2, 2};

// Stimulation power (percentage)
float stimPower[numBlocks] = {100, 75, 50, 25, 0, 25, 50, 75, 100};


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
boolean ledOn = true;

// containers for PWM
float maxPower = 255*75/100;
float currPower = 100; //% of maxPower
const int gatePin = 22;

String serialLine;

// -----     Functions     ----- //
void calculateTriggerDelay(int block) {
  // Convert Hz to us
  float stimPeriod = (1/stimFrequency[block])*1000000;
  // Calculate pulseWidth
  triggerDelay[block] = stimPeriod - triggerDelay[block];
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

void document(String parameter, float value) {
  Serial.print("data ");
  Serial.print(startTime);
  Serial.print(String(" ") + parameter + String(" "));
  Serial.println(value);
}

// LED and Pin control functions
void toggleLED() {
  if (ledOn) {
    turnLEDOff();
  } else {
    turnLEDOn();
  }
  if (activeExperiment) {
    scheduleNextLEDEvent();
  }
}

void turnLEDOff() {
  // Have to do something about setting power here...
  // digitalWrite alone will fail under certain PWM conditions
  pinMode(gatePin, OUTPUT);
  digitalWrite(gatePin, HIGH);
  document("led", 0);
  ledOn = false;
}

void turnLEDOn() {
  // Use a global for current power
  setLEDPower(currPower);
  document("led", currPower);
  ledOn = true;
}

// pulseWidth and triggerDelay in us
void scheduleNextLEDEvent() {
  if (ledOn) {
    nextLEDEvent = micros() + pulseWidth[currBlock];
  } else {
    nextLEDEvent = micros() + triggerDelay[currBlock];
  }
}

// PWM functions
void updateCurrPower() {
  currPower = stimPower[currBlock];
}

void setLEDPower(float power) {
  // Convert power (a percentage of maxPower) to dutycyle,
  debugOut(String("maxPower") + maxPower);
  int dc = 255 - (power/100)*maxPower;
  debugOut(String("dc: ") + dc);
  // write to pin
  pinMode(gatePin, OUTPUT);
  analogWrite(gatePin, dc);
}

// State control functions
void runStimulation() {
  logEvent("Starting Stimulation");
  activeExperiment = true;
  startTime = millis();
  startBlock(0);
}

void stopStimulation() {
  activeExperiment = false;
  turnLEDOff();
  currBlock = -1;
  document("stop", -1);
  logEvent("Stopped Stimulation");
}

void startBlock(int block) {
  logEvent(String("Starting Block ") + (block + 1));
  document("start", -1);
  // set current block to specified block
  currBlock = block;
  // update currPower
  updateCurrPower();
  // Schedule the ending for this block
  currBlockEnds = millis() + blockDuration[currBlock];
  // Start block by turning off all pins and scheduling next event
  // Blocks will always start on a trigger delay
  turnLEDOff();
  scheduleNextLEDEvent();
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
  switch (command[0]) {
      // 'S' means start
      case 'S':
        debugOut("Recieved Start Command");
        runStimulation();
        break;
      // 'X' means stop
      case 'X':
        debugOut("Recieved Stop Command");
        stopStimulation();
        break;
      // 'M' means enter manual mode
      case 'M':
        startManualMode();
        break;
      // 'P' is for POWAH
      case 'P':
        setLEDPower(arg);
        break;
      // 'O' is on
      case 'O':
        turnLEDOn();
        break;
      // 'F' is off
      case 'F':
        turnLEDOff();
        break;        
      case 'R':
        analogWriteFrequency(gatePin, arg);
        break;
      default:
        errOut("Unrecognized command!");
  }
}

// setup and loop
void setup(){
  // Set up PWM
  pinMode(gatePin, OUTPUT);
  analogWriteFrequency(gatePin, 200);
  turnLEDOff();
  
  // Convert block durations to milliseconds
  for (int i = 0; i < numBlocks; i++) {
    blockDuration[i] = blockDuration[i] * 1000;
  }
  
  // Convert Pulse Width to microseconds
  for (int i = 0; i < numBlocks; i++) {
    pulseWidth[i] = pulseWidth[i]*1000;
  }

  // Calculate Trigger Delays
  for (int i = 0; i < numBlocks; i++){
    calculateTriggerDelay(i);
  }

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
  if (activeExperiment && micros() >= nextLEDEvent) {
    toggleLED();
  }
}
