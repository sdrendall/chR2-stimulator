/* This is the set of functions used to parse and execute incoming commands from a 
  Serial connection.  The general framework may be useful for other projects but the commands
  themselves will have to be updated */
  
String serialLine;

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
    long int arg = argin.toInt();
  } else {
    errOut(String("Unexpected Input: \"") + input + String("\""));
  }
  executeCommand(command, arg);
}

void executeCommand(String command, long int arg) {
  switch (command) {
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
        initManualMode();
        break;
      // 'P' is for POWAH
      case 'P':
        writeValueToResistor(arg);
        break;
      // 'O' is on
      case 'O':
        turnPinsOn();
        break;
      // 'F' is off
      case 'F':
        turnPinsOff();
        break;        
      default:
        errOut("Unrecognized command!");
  }
}
