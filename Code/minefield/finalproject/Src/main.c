#include "rgb_led.h"

/*
 * Software CCR Implementation for RGB LED Control on STM32F3 Discovery Board
 * File: main.c
 */

// Function prototypes (defined in software_ccr.c)
void enable_clocks(void);
void initialise_board(void);
void timer_init(uint32_t time_period_ms, void (*callback)(void));
void enable_timer(void);
void set_rgb_color(void);
void set_rgb_ccr(uint8_t red, uint8_t green, uint8_t blue);

/**
 * @brief Main function
 * @retval int
 */
int main(void) {
    // Enable peripheral clocks for GPIO and Timers
    enable_clocks();

    // Initialize board and RGB LED pins
    initialise_board();

    // Initialize timer with a 1000ms period for color cycling
    // Each color in the rainbow sequence will display for 1 second
    timer_init(1000, set_rgb_color);


    // Start all timers (color cycling and PWM generation)
    enable_timer();

    // Main loop
    while(1) {
        // Everything is handled by the timer interrupts
    }

    // We never reach this point
    return 0;
}

