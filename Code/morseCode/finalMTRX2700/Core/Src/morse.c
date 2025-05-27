
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



void outputMorseData(uint8_t* morseStringData, uint8_t morseCompleteData) {
    uint8_t len = strlen((char*)morseStringData);
    uint8_t payload[len + 1];  // Morse bytes + 1-byte flag
    memcpy(payload, morseStringData, len);
    payload[len] = morseCompleteData;

    uint8_t packet[sizeof(Header) + len + 1];
    Header header = {
        .sentinel1 = SENTINEL_1,
        .sentinel2 = SENTINEL_2,
        .message_type = MORSE_MESSAGE,
        .data_length = len + 1
    };

    memcpy(packet, &header, sizeof(Header));
    memcpy(packet + sizeof(Header), payload, len + 1);

    SerialOutputBuffer(packet, sizeof(Header) + len + 1, &USART1_PORT);
}




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
	strings.newline = (uint8_t*)arrAlloc(2, sizeof(uint8_t)); // Allocate memory for newline message
	sprintf((char*)strings.newline, "\r\n"); // Newline message


	strings.fail = (uint8_t*)arrAlloc(39, sizeof(uint8_t)); // Allocate memory for fail message
	sprintf((char*)strings.fail, "\r\nLevel failed, please try again!\r\n"); // Fail message


	strings.completeLevel = (uint8_t*)arrAlloc(21, sizeof(uint8_t)); // Allocate memory for level complete message
	sprintf((char*)strings.completeLevel, "\r\nLevel complete!\r\n"); // Level complete message


	strings.completeGame = (uint8_t*)arrAlloc(20, sizeof(uint8_t)); // Allocate memory for game complete message
	sprintf((char*)strings.completeGame, "\r\nGame complete!\r\n"); // Game complete message


	// Setup arrays for the morse input messages
	strings.morseInput = (uint8_t**)arrAlloc(2, sizeof(uint8_t*));
	strings.morseInput[0] = (uint8_t*)arrAlloc(20, sizeof(uint8_t));
	strings.morseInput[1] = (uint8_t*)arrAlloc(20, sizeof(uint8_t));

	sprintf((char*)strings.morseInput[0], "Entered: dot\r\n"); // Dot input message
	sprintf((char*)strings.morseInput[1], "Entered: dash\r\n"); // Dash input message


	uint16_t totalLen = 0;

	// First calculate the total length
	for (uint8_t i = 0; i < morseLevels.numLevels + 1; i++) {
	    totalLen += strlen((char*)strings.levelDisplay[i]);
	}

	strings.totalMenu = (uint8_t*)arrAlloc(totalLen + 1, sizeof(uint8_t)); // +1 for null terminator
	strings.totalMenu[0] = '\0'; // Start with empty string

	for (uint8_t i = 0; i < morseLevels.numLevels + 1; i++) {
	    strcat((char*)strings.totalMenu, (char*)strings.levelDisplay[i]);
	}

	outputMorseData(strings.totalMenu, 0);

}




void initialiseFlags(void) {
	levelCompleteFlag = 0;
}


void assignMorse(uint8_t dotDash) {

	// User-provided input values
	uint8_t level = morseLog.currentLevel; // Set index for current level
	uint8_t index = morseLog.index[level]; // Set index for length of level code
	uint8_t withinLetterIndex = morseLog.letterIndex; // Set index for current position in letter

	// Comparison values as set in level initialisation
	uint8_t letterCode = morseLevels.levels[level][index].code; // Morse coded letter value
	uint8_t letterLength = morseLevels.levels[level][index].length; // Length of morse coded value
	uint8_t letter = morseLevels.levels[level][index].letter; // ASCII value of letter

	// Declare letter message
	strings.letter = (uint8_t*)arrAlloc(24, sizeof(uint8_t)); // Allocate memory for letter completed message
	sprintf((char*)strings.letter, "Letter completed: %c\r\n", letter); // Letter completed message

	morseLog.letter[withinLetterIndex] = dotDash; // 0 for dot, 1 for dash




	// Create singular strings for ease of transmission

	// Fail message
	// Compute required size
	size_t lenFail = strlen((char*)strings.fail)
			+ strlen((char*)strings.newline)
			+ strlen((char*)strings.levelDisplay[level + 1])
			+ strlen((char*)strings.newline)
			+ 1; // For null terminator

	// Allocate memory
	uint8_t* failString = (uint8_t*)arrAlloc(lenFail, sizeof(uint8_t));

	// Start with a copy of strings.fail
	strcpy((char*)failString, (char*)strings.fail);

	// Concatenate the rest
	strcat((char*)failString, (char*)strings.newline);
	strcat((char*)failString, (char*)strings.levelDisplay[level + 1]);
	strcat((char*)failString, (char*)strings.newline);




	// Level complete message
	// Compute required size
	size_t lenLevelComplete = strlen((char*)strings.completeLevel)
			+ strlen((char*)strings.newline)
			+ strlen((char*)strings.levelDisplay[level + 2])
			+ strlen((char*)strings.newline)
			+ 1; // For null terminator

	// Allocate memory
	uint8_t* levelCompleteString = (uint8_t*)arrAlloc(lenLevelComplete, sizeof(uint8_t));

	// Start with a copy of strings.completeLevel
	strcpy((char*)levelCompleteString, (char*)strings.completeLevel);

	// Concatenate the rest
	strcat((char*)levelCompleteString, (char*)strings.newline);
	strcat((char*)levelCompleteString, (char*)strings.levelDisplay[level + 2]);
	strcat((char*)levelCompleteString, (char*)strings.newline);




	// Letter complete message
	// Compute required size
	size_t lenLetterComplete = strlen((char*)strings.morseInput[dotDash])
			                  + strlen((char*)strings.newline)
	                          + strlen((char*)strings.letter)
	                          + 1;

	// Allocate memory
	uint8_t* letterCompleteString = (uint8_t*)arrAlloc(lenLetterComplete, sizeof(uint8_t));

	// Start with a copy of strings.fail
	strcpy((char*)letterCompleteString, (char*)strings.morseInput[dotDash]);

	// Concatenate the rest
	strcat((char*)letterCompleteString, (char*)strings.newline);
	strcat((char*)letterCompleteString, (char*)strings.letter);




	// Check if the correct input provided
	if (dotDash != ((letterCode >> ((letterLength - 1) - withinLetterIndex)) & 1)) {

		outputMorseData(failString, 0); // Serialise and output fail data

		morseLog.index[level] = 0;
		morseLog.letterIndex = 0;

		// Free dynamically allocated memory
		free(strings.letter);
		free(levelCompleteString);
		free(letterCompleteString);
		free(failString);
		return;
	}

	// Create morse letter out of currently stored morse code
	uint8_t fullLetter = 0;
	for (uint8_t i = 0; i <= withinLetterIndex; i++) {
	    fullLetter <<= 1; // Shift bits left as index increases
	    fullLetter |= (morseLog.letter[i] & 1); // Set each bit to 1 or 0 depending on dot/dash
	}


	outputMorseData(strings.morseInput[dotDash], 0); // Serialise and output dot/dash data

	// Check if full letter matches the provided value
	if ((fullLetter == letterCode) && (withinLetterIndex == (letterLength - 1))) {

		outputMorseData(letterCompleteString, 0); // Serialise and output letter data


		morseLog.letterIndex = 0; // Index back to the start of a new letter
		morseLog.index[level]++; // Increase the index variable for a new letter


		// Check if letter is the last in the level
		if (index == (morseLevels.size[level] - 1) && (level != (morseLevels.numLevels - 1))) {

			outputMorseData(levelCompleteString, 0); // Serialise and output level complete data

			morseLog.currentLevel++; // Increase the level number
			morseLog.index[level] = 0; // Reset the letter index
		}

		// Check if all levels are complete
		if ((level == (morseLevels.numLevels - 1)) && (index == (morseLevels.size[level] - 1))) {

			outputMorseData(strings.completeGame, 1); // Serialise and output game complete data

			// Stop button presses - wait for USART receive interrupt to finish program
	        EXTI->IMR &= ~EXTI_IMR_MR0; // Disable EXTI0 interrupt
		}

		// Free dynamically allocated memory
		free(strings.letter);
		free(levelCompleteString);
		free(letterCompleteString);
		free(failString);

		return;
	}


	// Free dynamically allocated memory
    free(strings.letter);
	free(levelCompleteString);
	free(letterCompleteString);
	free(failString);

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
uint8_t getLedState(void) {

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

	servoAngle(3, 50); // Set final servo angle

	// Free all dynamically allocated memory
	freeMatrix((void**)morseLevels.levels, morseLevels.numLevels);
	free(morseLevels.size);
	free(morseLog.index);
	free(morseLog.letter);

	freeMatrix((void**)strings.levelDisplay, (morseLevels.numLevels + 1));
	freeMatrix((void**)strings.morseInput, 2);
	free(strings.fail);
	free(strings.completeLevel);
	free(strings.completeGame);
	free(strings.totalMenu);

	while(1) {
		setLedState(0b01010101);
		delayMiliSec(1000); // 1000 ms delay
		setLedState(0b10101010);
		delayMiliSec(1000); // 1000 ms delay
	}
}
