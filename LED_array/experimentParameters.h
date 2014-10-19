#ifndef experimentParameters_h
#define experimentParameters_h

// These are the parameters for the current experiment
// THIS SHOULD BE THE ONLY FILE EDITED BY THE EXPERIMENTER!!!

// ----- User Defined Experiment Parameters -----//

// The number of blocks in the experiment.
// Each block represents a specific stimulation frequency and pulse width
// Over a specified duration
const int numBlocks = 1;

// The number of LEDs on the array (typically 12)
const int numLEDs = 6;
const int firstPin = 4;

// Block Parameters.  Each parameter consists of a list of values corresponding to the
// value of that parameter in each respective block.
// THE NUMBER OF ENTRIES IN EACH LIST MUST BE EQUAL TO numBlocks!

// Block durations, in MILLISECONDS!
const unsigned long blockDuration[numBlocks] = {7200000};


// ----- Stimulation Parameters ----- //

// The LED array control software is designed to drive bursts of LED pulses.
// Henceforth, one cycle during which the LED is turned on, then off is refered to as a 'pulse'.
// A burst consists of a train of pulses at a given frequency, with a constant, user defined pulse width.
//
// Each of the following variables is a numLEDs x numBlocks array.  
// Descending rows correspond to subsequent LEDs.
// Subsequent columns correspond to subsequent blocks.


// The frequency of pulses during each burst - in Hertz
const float exp_pulseFreq[numLEDs][numBlocks] = {
    {10},
    {10},
    {10},
    {10},
    {10},
    {10}
};

// The pulse width of each pulse delivered in a burst - in milliseconds
const unsigned long exp_pulseWidth[numLEDs][numBlocks] = {
    {5},
    {5},
    {5},
    {5},
    {5},
    {5}
};

// Percentage of maximum power to drive each LED with - in %
const unsigned int exp_stimPower[numLEDs][numBlocks] = {
    {100},
    {100},
    {100},
    {100},
    {100},
    {100}
};

// The bursting frequency - in Hertz 
const float exp_burstFreq[numLEDs][numBlocks] = {
    {.0033},
    {.0033},
    {.0033},
    {.0033},
    {.0033},
    {.0033}
};

// The burst duration - in milliseconds
const unsigned long exp_burstDuration[numLEDs][numBlocks] = {
    {300000},
    {300000},
    {300000},
    {300000},
    {300000},
    {300000}    
    };
#endif