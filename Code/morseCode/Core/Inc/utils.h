// utils.h

#ifndef UTILS_H
#define UTILS_H

// Includes if needed
#include <stdint.h>


// Store a morse encoded letter
typedef struct {
	uint8_t letter;
    uint8_t code; // binary-encoded Morse (1 for dash, 0 for dot)
    uint8_t length; // number of Morse units
} MorseBin;


extern MorseBin morseDict[];


// Store all level data and parameters
typedef struct {
    MorseBin **levels;            // Stores encoded characters (dots and dashes)
    uint8_t *size;               // Store individual size of each level
    uint8_t numLevels;            // Store the total number of levels
    uint8_t maxLevelSize;            // Store the max size of each level
    uint8_t maxMorseLetterLength; // Store the maximum length of a morse-coded letter
} MorseLevels;


// Store all user data
typedef struct {
    uint8_t *index;       // Number of valid entries
    uint8_t currentLevel; // Current level in play
    uint8_t *letter;      // Current letter being written
    uint8_t letterIndex;  // Index of letter being written
} MorseLog;


// Store all user strings
typedef struct {
	uint8_t **levelDisplay;   // Displays all levels
    uint8_t **morseInput;     // Stores input
    uint8_t *fail;            // Stores failed message
    uint8_t *letter;          // Stores letter message
    uint8_t *completeLevel;   // Stores level complete message
    uint8_t *completeGame;    // Stores game complete message
} Strings;


void initialiseMorse(void);

void initialiseLevelCompleteFlag (void);

void assignMorse(uint8_t dotDash);

void delayMiliSec(uint16_t ms);

// Function to get the LED state
uint8_t getLedState();

// Function to set the LED state
void setLedState(uint8_t state);

uint8_t getLevelCompleteFlag(void);

void setLevelCompleteFlag(void);

void levelCompleteProcedure(void);


#endif // UTILS_H
