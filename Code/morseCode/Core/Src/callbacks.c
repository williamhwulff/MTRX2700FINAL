
#include "main.h"



// Create delay after finishing transmission
void finishedTransmissionCallback(void) {
    TIM2->CCR1 = 0; // Red on 0
    TIM2->CCR3 = 0; // Green on full

    delayMiliSec(10); // 10 ms delay

}

uint8_t EXTI0Callback(uint8_t messageSent) {
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

        messageSent = 0; // Reset message as unsent

    } else {
        // FALLING edge - button released
        TIM3->CR1 &= ~TIM_CR1_CEN; // Stop timer
        uint16_t duration = TIM3->CNT; // Record the counter value to determine if dot or dash

        if (duration < 250 && messageSent == 0) {
            // Dot
            assignMorse(0); // Assign a 0 to the struct for a dot

            messageSent = 1; // Set value of message as sent

            TIM2->CCR1 = 10000; // Red on full
            TIM2->CCR3 = 0; // Green on 0

        } else if (messageSent == 0) {
            // Dash
            assignMorse(1); // Assign a 1 to the struct for a dash

            messageSent = 1; // Set value of message as sent


            TIM2->CCR1 = 0; // Red on 0
            TIM2->CCR3 = 10000; // Green on full
        }
    }

	return messageSent; // Return for use in interrupt file
}


uint8_t TIM3Callback(uint8_t messageSent) {
	TIM3->CR1 &= ~TIM_CR1_CEN;  // Clear the CEN bit to stop the timer (prevent regular interval interrupts)

	assignMorse(1); // Assign a 1 to the struct for a dash

	messageSent = 1; // Set value of message as sent

	TIM2->CCR1 = 0; // Red on 0
	TIM2->CCR3 = 10000; // Green on full

	return messageSent; // Return for use in interrupt file
}
