/*
 * servo.c
 *
 *  Created on: May 22, 2025
 *      Author: willw
 */

#include "main.h"


void setupGPIOPinsTim8(void) {
	// Setup GPIO for PWM output
	GPIOC->MODER |= (0x2 << (6 * 2)) | (0x2 << (7 * 2)); // Alternate function mode for pins 6, 7 CH(1, 2)
	GPIOC->MODER |= (0x2 << (8 * 2)) | (0x2 << (9 * 2)); // Alternate function mode for pins 8, 9 CH(3, 4)

	GPIOC->AFR[0] |= (0x4 << (6 * 4)) | (0x4 << (7 * 4)); // Set alternate functions for pins 6, 7
	GPIOC->AFR[1] |= (0x4 << ((8 - 8) * 4)) | (0x4 << ((9 - 8) * 4)); // Set alternate function for pins 8, 9
}


void setupTim8Pwm(void) {

	RCC->APB2ENR |= RCC_APB2ENR_TIM8EN; // Enable TIM8 peripheral clock
	TIM8->BDTR |= TIM_BDTR_MOE; // Enable outputs for timer 8

	// Configure timer registers
	TIM8->PSC = 47; // Prescaler of 1 microsec for 8 MHz clock
	TIM8->ARR = 20000; // Auto-reload after 20 ms for PWM application

	// Set to PWM mode 1
	TIM8->CCMR1 |= (0x6 << 4) | TIM_CCMR1_OC1PE | (0x6 << 12) | TIM_CCMR1_OC2PE; // PWM mode 1 for Channel 1 and 2
	TIM8->CCMR2 |= (0x6 << 4) | TIM_CCMR2_OC3PE | (0x6 << 12) | TIM_CCMR2_OC4PE; // PWM mode 1 for Channel 3 and 4


	// Set and enable polarity for all channels
	TIM8->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P | TIM_CCER_CC3P | TIM_CCER_CC4P);
	TIM8->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;


	TIM8->CR1 |= TIM_CR1_ARPE; // Enable auto-reload preload
	TIM8->EGR = TIM_EGR_UG; // Generate an update event (apply PSC + ARR)

	TIM8->CNT = 0;
	TIM8->CR1 |= TIM_CR1_CEN; // Start the timer

}


void servoAngle(uint8_t channel, uint8_t degree) {
	// Calculate the pulse width
    uint16_t pulse = (uint16_t)(1000 + (1000 * degree)/90);

    // Apply to the given channel
    switch (channel) {
        case 1: TIM8->CCR1 = pulse; break;
        case 2: TIM8->CCR2 = pulse; break;
        case 3: TIM8->CCR3 = pulse; break;
        case 4: TIM8->CCR4 = pulse; break;
    }
}
