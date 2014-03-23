// These are the parameters for the current experiment
// THIS SHOULD BE THE ONLY FILE EDITED BY THE EXPERIMENTER!!!

// ----- User Defined Experiment Parameters -----//

// The number of blocks in the experiment.
// Each block represents a specific stimulation frequency and pulse width
// Over a specified duration
const int numBlocks = 9;

// The number of LEDs on the array (typically 12)
const int numLEDs = 12;

// Block Parameters.  Each parameter consists of a list of values corresponding to the
// value of that parameter in each respective block.
// THE NUMBER OF ENTRIES IN EACH LIST MUST BE EQUAL TO numBlocks!

// Block durations, in seconds
unsigned long blockDuration[numBlocks] = {5, 5, 5, 5, 5, 5, 5, 5, 5};


// ----- Stimulation Parameters ----- //

// The LED array control software is designed to drive bursts of LED pulses.
// Henceforth, one cycle during which the LED is turned on, then off is refered to as a 'pulse'.
// A burst consists of a train of pulses at a given frequency, with a constant, user defined pulse width.
//
// Each of the following variables is a numLEDs x numBlocks array.  
// Descending rows correspond to subsequent LEDs.
// Subsequent columns correspond to subsequent blocks.


// The frequency of pulses during each burst - in Hertz
float exp_pulseFreq[numLEDs][numBlocks] = {
    {.25, .5, 1, 2, 4},
    {.25, .5, 1, 2, 4},
    {.25, .5, 1, 2, 4},
    {.25, .5, 1, 2, 4},
    {.25, .5, 1, 2, 4},
    {.25, .5, 1, 2, 4},
    {.25, .5, 1, 2, 4},
    {.25, .5, 1, 2, 4},
    {.25, .5, 1, 2, 4},
    {.25, .5, 1, 2, 4},
    {.25, .5, 1, 2, 4},
    {.25, .5, 1, 2, 4},
};

// The pulse width of each pulse delivered in a burst - in milliseconds
unsigned long exp_pulseWidth[numLEDs][numBlocks] = {
    {2, 2, 2, 2, 2},
    {2, 2, 2, 2, 2},
    {2, 2, 2, 2, 2},
    {2, 2, 2, 2, 2},
    {2, 2, 2, 2, 2},
    {2, 2, 2, 2, 2},
    {2, 2, 2, 2, 2},
    {2, 2, 2, 2, 2},
    {2, 2, 2, 2, 2},
    {2, 2, 2, 2, 2},
    {2, 2, 2, 2, 2},
    {2, 2, 2, 2, 2}
};

// The bursting frequency - in Hertz 
float exp_burstFreq[numLEDs][numBlocks] = {
    {.1, .1, .1, .1, .1},
    {.1, .1, .1, .1, .1},
    {.1, .1, .1, .1, .1},
    {.1, .1, .1, .1, .1},
    {.1, .1, .1, .1, .1},
    {.1, .1, .1, .1, .1},
    {.1, .1, .1, .1, .1},
    {.1, .1, .1, .1, .1},
    {.1, .1, .1, .1, .1},
    {.1, .1, .1, .1, .1},
    {.1, .1, .1, .1, .1},
    {.1, .1, .1, .1, .1}
};

// The burst duration - in milliseconds
unsigned long exp_burstDuration[numLEDs][numBlocks] = {
    {800, 800, 800, 800, 800},
    {800, 800, 800, 800, 800},
    {800, 800, 800, 800, 800},
    {800, 800, 800, 800, 800},
    {800, 800, 800, 800, 800},
    {800, 800, 800, 800, 800},
    {800, 800, 800, 800, 800},
    {800, 800, 800, 800, 800},
    {800, 800, 800, 800, 800},
    {800, 800, 800, 800, 800},
    {800, 800, 800, 800, 800},
    {800, 800, 800, 800, 800}
};