
#include "main.h"



// Create delay after finishing transmission
void finishedTransmissionCallback(void) {
    TIM2->CCR1 = 0; // Red on 0
    TIM2->CCR3 = 0; // Green on full

    delayMiliSec(10); // 10 ms delay

}

uint8_t morseEXTI0Callback(uint8_t messageSent) {

	uint8_t message; // Declare message to return

	if (GPIOA->IDR & 0x01) {

		// RISING edge - button pressed
		TIM3->CNT = 0; // Reset count
		TIM3->CR1 |= TIM_CR1_CEN; // Start timer

        // Debouncing by disabling interrupts, running delay and re-enabling
        EXTI->IMR &= ~EXTI_IMR_MR0; // Disable EXTI0 interrupt

        // Debounce delay
        while (TIM3->CNT < 20) {}

        // Clear any spurious pending bit again
        EXTI->PR |= EXTI_PR_PR0;

        EXTI->IMR |= EXTI_IMR_MR0;  // Re-enable EXTI0 interrupt

        message = 0; // Reset message as unsent

    } else {
        // FALLING edge - button released
        TIM3->CR1 &= ~TIM_CR1_CEN; // Stop timer
        uint16_t duration = TIM3->CNT; // Record the counter value to determine if dot or dash

        if (duration < 250 && messageSent == 0) {
            // Dot
            assignMorse(0); // Assign a 0 to the struct for a dot

            message = 1; // Set value of message as sent

            TIM2->CCR1 = 10000; // Red on full
            TIM2->CCR3 = 0; // Green on 0

        } else if (messageSent == 0) {
            // Dash
            assignMorse(1); // Assign a 1 to the struct for a dash

            message = 1; // Set value of message as sent


            TIM2->CCR1 = 0; // Red on 0
            TIM2->CCR3 = 10000; // Green on full
        }
    }

	return message; // Return for use in interrupt file
}

void morseEXTI1Callback(void) {

	// Initialise morse code module - call this to start the module
	delayMiliSec(1000); // Add delay for startup
	initialiseFlags(); // Initialise the challenge as incomplete
	initialiseMorse(); // Initialise the struct to store morse code values

    EXTI->IMR &= ~EXTI_IMR_MR1; // Disable EXTI1 interrupt
    EXTI->IMR |= EXTI_IMR_MR0;  // Enable EXTI0 interrupt (allow program to start)


}

void morseEXTI4Callback(void) {

    EXTI->IMR &= ~EXTI_IMR_MR4; // Disable EXTI1 interrupt
    servoAngle(2, 90); // Set final servo position


}

uint8_t TIM3Callback(uint8_t message) {
	TIM3->CR1 &= ~TIM_CR1_CEN;  // Clear the CEN bit to stop the timer (prevent regular interval interrupts)

	assignMorse(1); // Assign a 1 to the struct for a dash

	message = 1; // Set value of message as sent

	TIM2->CCR1 = 0; // Red on 0
	TIM2->CCR3 = 10000; // Green on full

	return message; // Return for use in interrupt file
}
