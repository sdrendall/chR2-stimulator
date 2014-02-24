
// ----- User Defined Experiment Parameters -----//

// The number of blocks in the experiment.
// Each block represents a specific stimulation frequency and pulse width
// Over a specified duration
const int numBlocks = 4;

// Block Parameters.  Each parameter consists of a list of values corresponding to the
// value of that parameter in each respective block.
// THE NUMBER OF ENTRIES IN EACH LIST MUST BE EQUAL TO numBlocks!

// Boolean.  0 indicates no stimulation.
// pulseWidth, stimFrequency and stimPower will be set automatically
// if stimulate[block] == 0 
boolean stimulate[numBlocks] = {1, 0, 1, 1};

// Block durations, in seconds
unsigned long blockDuration[numBlocks] = {2, 3, 3, 4};

// Pulse width in milliseconds
unsigned long pulseWidth[numBlocks] = {200, 0, 500, 250};

// Stimulation Frequency in Hz
unsigned long stimFrequency[numBlocks] = {1, 0, 1, 2};

// Stimulation power (percentage)
float stimPower[numBlocks] = {0, 0, 20, 100};


// Pins, specify first and last pin to be used.
const int firstPin = 13;
const int lastPin = 13;



// ----- Program Parameters (do not edit) -----//
int triggerDelay[numBlocks];

int currBlock = -1;
unsigned long currBlockEnds;
unsigned long nextPinEvent;

boolean activeExperiment = false;
boolean pinsOn = false;
boolean stimulating = 0;


const int numPins = lastPin - firstPin + 1;
int PINS[numPins];

//For Debugging
unsigned long stimPeriod[numBlocks];

// -----     Functions     -----//
void calculateTriggerDelay(int block) {
  // Convert Hz to ms
  stimPeriod[block] = (1/stimFrequency[block])*1000;
  // Calculate triggerDelay
  triggerDelay[block] = stimPeriod - pulseWidth[block];
}
  
void logEvent(String msg) {
  Serial.print("[TIME]: ");
  Serial.print(millis());
  Serial.print(" [LOG]: ");
  Serial.println(msg);
}


void updatePins() {
  if (pinsOn) {
    turnPinsOff();
  } else {
    turnPinsOn();
  }
  scheduleNextPinEvent();
}


void turnPinsOff() {
  for(int i = 0; i < numPins; i++) {
    digitalWrite(PINS[i], LOW);
  }
  pinsOn = false;  
}


void turnPinsOn() {
  for (int i = 0; i < numPins; i++) {
    digitalWrite(PINS[i], HIGH);
  }
  pinsOn = true;
}


// pulseWidth and triggerDelay in ms
void scheduleNextPinEvent() {
  if (pinsOn) {
    nextPinEvent = micros()/1000 + pulseWidth[currBlock];
  } else {
    nextPinEvent = micros()/1000 + triggerDelay[currBlock];
  }
}


void runStimulation() {
  logEvent("Starting Stimulation");
  activeExperiment = true;
  startBlock(0);
}

void stopStimulation() {
  activeExperiment = false;
  stimulating = false;
  turnPinsOff();
  currBlock = -1;
  logEvent("Stopped Stimulation");
}  


void startBlock(int block) {
  logEvent(String("Starting Block ") + block);
  // set current block to specified block
  currBlock = block;
  // check if I should stimulate
  if (stimulate[currBlock]) {
    stimulating = true;
  } else {
    stimulating = false;
  }
  // Schedule the ending for this block
  currBlockEnds = millis() + blockDuration[currBlock];
  // Start block by turning off all pins and scheduling next event
  // Blocks will always start on a trigger delay
  turnPinsOff();
  scheduleNextPinEvent();
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
  
void setup(){
  // Specify pins as outputs
  for(int i = 0; i < numPins; i++){
    PINS[i] = i + firstPin;
    pinMode(PINS[i], OUTPUT);
  }
  // Convert block durations to milliseconds
  for (int i = 0; i < numBlocks; i++) {
    blockDuration[i] = blockDuration[i] * 1000;
  }
  
  // Overwrite block parameters where !stimulate
  for (int i = 0; i < numBlocks; i++){
    if (!stimulate[i]) {
      pulseWidth[i] = 0;
      stimFrequency[i] = 1/blockDuration[i];
      stimPower[i] = 0;
    }
  }
  
  // Calculate Trigger Delays
  for (int i = 0; i < numBlocks; i++){
    calculateTriggerDelay(i);
  }

  // Initialize serial communication
  Serial.begin(9600);
  
  // Print stim periods.  For Debugging
for (int i = 0; i < numBlocks; i++) {
  debugOut(String("Stimulation Period[") + i + String("] = ") + stimPeriod[i]);
} 
  
  // Until start conditions are added
  runStimulation();
}
  

void loop() {
  // Set up something to include start conditions.  Button Press?
  
  // Update block when scheduled
  if (activeExperiment && currBlock <= millis()) {
    startNextBlock();
  }
    
  // Check Pins when scheduled
  if (stimulating && (micros()/1000 >= nextPinEvent)) {
    updatePins();
  }
}
