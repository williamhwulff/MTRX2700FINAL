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
	// Setup GPIO for PWM output
	GPIOA->MODER = 0;
	GPIOA->MODER |= (0x2 << (15 * 2)) | (0x2 << (1 * 2)); // Alternate function mode for pins 15, 1 CH(1, 2)
	GPIOA->MODER |= (0x2 << (2 * 2)) | (0x2 << (3 * 2)); // Alternate function mode for pins 2, 3 CH(3, 4)

	GPIOA->AFR[0] |= (0x1 << (1 * 4)) | (0x1 << (2 * 4)) | (0x1 << (3 * 4)); // Set alternate functions for pins 1, 2, 3
	GPIOA->AFR[1] |= (0x1 << ((15 - 8) * 4)); // Set alternate function for pin 15
}


void setupTim2Pwm(void) {

	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // Enable TIM2 peripheral clock

	// Configure timer registers
	TIM2->PSC = 7; // Prescaler of 1 microsec for 8 MHz clock
	TIM2->ARR = 20000; // Auto-reload after 20 ms for PWM application

	// Set to PWM mode 1
	TIM2->CCMR1 |= (0x6 << 4) | TIM_CCMR1_OC1PE | (0x6 << 12) | TIM_CCMR1_OC2PE; // PWM mode 1 for Channel 1 and 2
	TIM2->CCMR2 |= (0x6 << 4) | TIM_CCMR2_OC3PE | (0x6 << 12) | TIM_CCMR2_OC4PE; // PWM mode 1 for Channel 3 and 4


	// Set and enable polarity for all channels
	TIM2->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P | TIM_CCER_CC3P | TIM_CCER_CC4P);
	TIM2->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;


	TIM2->CR1 |= TIM_CR1_ARPE; // Enable auto-reload preload
	TIM2->EGR = TIM_EGR_UG; // Generate an update event (apply PSC + ARR)

	TIM2->CNT = 0;
	TIM2->CR1 |= TIM_CR1_CEN; // Start the timer

}


void servoAngle(uint8_t channel, float degree) {
	// Calculate the pulse width
    uint16_t pulse = (uint16_t)(1000 + 1000 * (degree / 90.0f));

    // Apply to the given channel
    switch (channel) {
        case 1: TIM2->CCR1 = pulse; break;
        case 2: TIM2->CCR2 = pulse; break;
        case 3: TIM2->CCR3 = pulse; break;
        case 4: TIM2->CCR4 = pulse; break;
    }
}
