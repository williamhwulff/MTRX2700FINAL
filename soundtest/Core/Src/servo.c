/*
 * servo.c
 *
 *  Created on: May 22, 2025
 *      Author: willw
 */

#include "main.h"

void enableClocks(void) {
	// Enable all GPIO clocks
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN | RCC_AHBENR_GPIOEEN;
}

void setupGPIOPinsTim2(void) {
    // Setup GPIO for PWM output: CH1=PA15, CH2=PA1, CH4=PA3 (drop CH3/PA2)
    GPIOA->MODER |= (0x2 << (15 * 2))  // PA15 AF
                   | (0x2 << (1 * 2))  // PA1  AF
                   | (0x2 << (3 * 2)); // PA3  AF

    // Alternate‐function select for PA15, PA1, PA3
    GPIOA->AFR[0] |= (0x1 << (1 * 4))  // PA1 → AF1 (TIM2_CH2)
                   | (0x1 << (3 * 4)); // PA3 → AF1 (TIM2_CH4)
    GPIOA->AFR[1] |= (0x1 << ((15 - 8) * 4)); // PA15 → AF1 (TIM2_CH1)
}

void setupTim2Pwm(void) {

	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // Enable TIM2 peripheral clock

	// Configure timer registers
	TIM2->PSC = 7; // Prescaler of 1 microsec for 8 MHz clock
	TIM2->ARR = 20000; // Auto-reload after 20 ms for PWM application

    // Set to PWM mode 1 for CH1, CH2, CH4 only (drop CH3)
    TIM2->CCMR1 |= (0x6 << 4) | TIM_CCMR1_OC1PE    // CH1
                 | (0x6 << 12) | TIM_CCMR1_OC2PE;  // CH2
    TIM2->CCMR2 |=               /* no CH3 */
                 (0x6 << 12) | TIM_CCMR2_OC4PE;  // CH4

    // Enable polarity & output for CH1, CH2, CH4
    TIM2->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P /*| CC3P*/ | TIM_CCER_CC4P);
    TIM2->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E           | TIM_CCER_CC4E;


	TIM2->CR1 |= TIM_CR1_ARPE; // Enable auto-reload preload
	TIM2->EGR = TIM_EGR_UG; // Generate an update event (apply PSC + ARR)

	TIM2->CNT = 0;
	TIM2->CR1 |= TIM_CR1_CEN; // Start the timer

}


void servoAngle(uint8_t channel, uint8_t degree) {
	// Calculate the pulse width
    uint16_t pulse = (uint16_t)(1000 + (1000 * degree)/90);

    // Apply to the given channel
    switch (channel) {
        case 1: TIM2->CCR1 = pulse; break;
        case 2: TIM2->CCR2 = pulse; break;
        case 3: TIM2->CCR3 = pulse; break;
        case 4: TIM2->CCR4 = pulse; break;
    }
}
