
#include "main.h"

MorseBin morseDict[] = {
    {'A', 0b01, 2},    // .-
    {'B', 0b1000, 4},  // -...
    {'C', 0b1010, 4},  // -.-.
    {'D', 0b100, 3},   // -..
    {'E', 0b0, 1},     // .
    {'F', 0b0010, 4},  // ..-.
    {'G', 0b110, 3},   // --.
    {'H', 0b0000, 4},  // ....
    {'I', 0b00, 2},    // ..
    {'J', 0b0111, 4},  // .---
    {'K', 0b101, 3},   // -.-
    {'L', 0b0100, 4},  // .-..
    {'M', 0b11, 2},    // --
    {'N', 0b10, 2},    // -.
    {'O', 0b111, 3},   // ---
    {'P', 0b0110, 4},  // .--.
    {'Q', 0b1101, 4},  // --.-
    {'R', 0b010, 3},   // .-.
    {'S', 0b000, 3},   // ...
    {'T', 0b1, 1},     // -
    {'U', 0b001, 3},   // ..-
    {'V', 0b0001, 4},  // ...-
    {'W', 0b011, 3},   // .--
    {'X', 0b1001, 4},  // -..-
    {'Y', 0b1011, 4},  // -.--
    {'Z', 0b1100, 4}   // --..
};

MorseLevels morseLevels; // Create struct to log all level information
MorseLog morseLog;       // Create struct to log all user information
Strings strings;           // Create struct to store all strings

uint8_t levelCompleteFlag; // Flag signals if challenge complete


void initialiseMorse(void) {

	// First declare level parameters
	morseLevels.numLevels = 3;
	morseLevels.maxLevelSize = 8;
	morseLevels.maxMorseLetterLength = 4;

	// Next allocate space for the level data and their sizes
	morseLevels.levels = (MorseBin**)matrixAlloc(morseLevels.numLevels, morseLevels.maxLevelSize, sizeof(MorseBin));
	morseLevels.size = (uint8_t*)arrAlloc(morseLevels.numLevels, sizeof(uint8_t));

	// Level 1 "SOS"
	morseLevels.levels[0][0] = morseDict[18];
	morseLevels.levels[0][1] = morseDict[14];
	morseLevels.levels[0][2] = morseDict[18];
	morseLevels.size[0] = 3;

	// Level 2 "HELP"
	morseLevels.levels[1][0] = morseDict[7];
	morseLevels.levels[1][1] = morseDict[4];
	morseLevels.levels[1][2] = morseDict[11];
	morseLevels.levels[1][3] = morseDict[15];
	morseLevels.size[1] = 4;

	// Level 3 "SAVE KHIT"
	morseLevels.levels[2][0] = morseDict[18];
	morseLevels.levels[2][1] = morseDict[0];
	morseLevels.levels[2][2] = morseDict[21];
	morseLevels.levels[2][3] = morseDict[4];

	morseLevels.levels[2][4] = morseDict[10];
	morseLevels.levels[2][5] = morseDict[7];
	morseLevels.levels[2][6] = morseDict[8];
	morseLevels.levels[2][7] = morseDict[19];
	morseLevels.size[2] = 8;

	morseLog.index = (uint8_t*)arrAlloc(morseLevels.numLevels, sizeof(uint8_t));
	morseLog.letter = (uint8_t*)arrAlloc(morseLevels.maxMorseLetterLength, sizeof(uint8_t));
	morseLog.currentLevel = 0;
	morseLog.letterIndex = 0;


	// Create strings for user interface

	// Create level display header
	strings.levelDisplay = (uint8_t**)arrAlloc((morseLevels.numLevels + 1), sizeof(uint8_t*));
	strings.levelDisplay[0] = (uint8_t*)arrAlloc(28, sizeof(uint8_t));

	sprintf((char*)strings.levelDisplay[0], "\r\nLevels to be cleared:\r\n\r\n");

	// Fill in each level line
	for (uint8_t i = 0; i < morseLevels.numLevels; i++) {
		strings.levelDisplay[i + 1] = (uint8_t*)arrAlloc(12 + (2 * morseLevels.size[i]), sizeof(uint8_t));

	    sprintf((char*)strings.levelDisplay[i + 1], "Level %d: ", i + 1); // Start with the level name

	    // Append each letter to the existing line
	    for (uint8_t j = 0; j < morseLevels.size[i]; j++) {
	        char letterStr[4];  // Enough space for letter + space + null
	        sprintf((char*)letterStr, "%c ", morseLevels.levels[i][j].letter);
	        strcat((char*)strings.levelDisplay[i + 1], letterStr);  // Append to current line
	    }

	    strcat((char*)strings.levelDisplay[i + 1], "\r\n");  // End the line
	}

	// Create other messages
	strings.fail = (uint8_t*)arrAlloc(39, sizeof(uint8_t)); // Allocate memory for fail message
	sprintf((char*)strings.fail, "\r\nLevel failed, please try again!\r\n"); // Fail message

	strings.completeLevel = (uint8_t*)arrAlloc(21, sizeof(uint8_t)); // Allocate memory for level complete message
	sprintf((char*)strings.completeLevel, "\r\nLevel complete!\r\n"); // Level complete message

	strings.completeGame = (uint8_t*)arrAlloc(20, sizeof(uint8_t)); // Allocate memory for game complete message
	sprintf((char*)strings.completeGame, "\r\nGame complete!\r\n"); // Game complete message

	// Setup arrays for the morse input messages
	strings.morseInput = (uint8_t**)arrAlloc(2, sizeof(uint8_t*));
	strings.morseInput[0] = (uint8_t*)arrAlloc(17, sizeof(uint8_t));
	strings.morseInput[1] = (uint8_t*)arrAlloc(18, sizeof(uint8_t));

	sprintf((char*)strings.morseInput[0], "\r\nEntered: dot\r\n"); // Dot input message
	sprintf((char*)strings.morseInput[1], "\r\nEntered: dash\r\n"); // Dash input message


}


void initialiseLevelCompleteFlag (void) {
	levelCompleteFlag = 0;
}


void assignMorse(uint8_t dotDash) {

	// User-provided input values
	uint8_t level = morseLog.currentLevel; // set index for current level
	uint8_t index = morseLog.index[level]; // Set index for length of level code
	uint8_t withinLetterIndex = morseLog.letterIndex; // Set index for current position in letter

	// Comparison values as set in level initialisation
	uint8_t letterCode = morseLevels.levels[level][index].code; // Morse coded letter value
	uint8_t letterLength = morseLevels.levels[level][index].length; // Length of morse coded value
	uint8_t letter = morseLevels.levels[level][index].letter; // ASCII value of letter

	// Declare letter message
	strings.letter = (uint8_t*)arrAlloc(26, sizeof(uint8_t)); // Allocate memory for letter completed message
	sprintf((char*)strings.letter, "\r\nLetter completed: %c\r\n", letter); // Letter completed message

	morseLog.letter[withinLetterIndex] = dotDash; // 0 for dot, 1 for dash

	// Check if the correct input provided
	if (dotDash != ((letterCode >> ((letterLength - 1) - withinLetterIndex)) & 1)) {
		SerialOutputChar('F', &USART1_PORT); // Output the fail code on the USART line
		morseLog.index[level] = 0;
		morseLog.letterIndex = 0;
		return;
	}

	// Create morse letter out of currently stored morse code
	uint8_t fullLetter = 0;
	for (uint8_t i = 0; i <= withinLetterIndex; i++) {
	    fullLetter <<= 1; // Shift bits left as index increases
	    fullLetter |= (morseLog.letter[i] & 1); // Set each bit to 1 or 0 depending on dot/dash
	}


	SerialOutputChar(dotDash, &USART1_PORT); // Output the code on the USART line

	// Check if full letter matches the provided value
	if ((fullLetter == letterCode) && (withinLetterIndex == (letterLength - 1))) {

		SerialOutputChar(letter, &USART1_PORT); // Output code for letter

		morseLog.letterIndex = 0; // Index back to the start of a new letter
		morseLog.index[level]++; // Increase the index variable for a new letter


		// Check if letter is the last in the level
		if (index == (morseLevels.size[level] - 1)) {
			SerialOutputChar('Y', &USART1_PORT); // Output the fail code on the USART line
			morseLog.currentLevel++; // Increase the level number
			morseLog.index[level] = 0; // Reset the letter index
		}

		// Check if all levels are complete
		if (level == (morseLevels.numLevels - 1)) {
			SerialOutputChar('W', &USART1_PORT); // Output the fail code on the USART line
		}

		return;
	}



	morseLog.letterIndex++; // Increase index for dot/dash within the letter
}

void delayMiliSec(uint16_t ms) {
	volatile uint32_t* timerCnt = &(TIM4->CNT);


	TIM4->CNT = 0;
    TIM4->CR1 |= TIM_CR1_CEN; // Start timer

    while (*timerCnt < ms) {
        // Wait (busy loop)
    }

    TIM4->CR1 &= ~TIM_CR1_CEN; // Stop timer
}

// Function to get the LED state
uint8_t getLedState() {

	// Update LED display
	uint8_t *led_register = ((uint8_t*)&(GPIOE->ODR)) + 1;

	return *led_register;
}

// Function to set the LED state
void setLedState(uint8_t state) {
    uint8_t ledState = state;

    // Update LED display
    uint8_t *led_register = ((uint8_t*)&(GPIOE->ODR)) + 1;
    *led_register = ledState;
}

uint8_t getLevelCompleteFlag(void) {
	return levelCompleteFlag;
}

void setLevelCompleteFlag(void) {
	levelCompleteFlag = 1;
}

void levelCompleteProcedure(void) {
	TIM2->CCR2 = 2000; // Set final servo position (let ball slide through)

	// Free all dynamically allocated memory
	freeMatrix((void**)morseLevels.levels, morseLevels.numLevels);
	free(morseLevels.size);
	free(morseLog.index);
	free(morseLog.letter);
	while(1) {
		setLedState(0b01010101);
		delayMiliSec(1000); // 1000 ms delay
		setLedState(0b10101010);
		delayMiliSec(1000); // 1000 ms delay
	}
}
