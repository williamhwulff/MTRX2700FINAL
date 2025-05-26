/*
 * This is a header file you might need to create in your project
 * to declare the functions used in a.c and main.c
 * File: rgb_led.h
 */
#ifndef RGB_LED_H
#define RGB_LED_H

#define ALTFUNCTION 0xA00
#define RXTX 0x770000
#define HIGHSPEED 0xF00
#define BAUDRATE 0x46


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "stm32f303xc.h"
#include <stdlib.h>
#include <time.h>

#include "a.h"
// Function prototypes
void enable_clocks(void);
void initialise_board(void);
void timer_init(uint32_t time_period_ms, void (*callback)(void));
void set_rgb_color(void);
void enable_timer(void);

void set_rgb2_ccr(uint8_t red, uint8_t green, uint8_t blue);
void set_rgb3_ccr(uint8_t red, uint8_t green, uint8_t blue);
void set_rgb4_ccr(uint8_t red, uint8_t green, uint8_t blue);
void test_rgb2(void);
void test_rgb3(void);
void test_rgb4(void);

void configure_dac_random(void);
uint8_t get_random_led_index(void);
void randomize_yellow_led(void);
void trigger_randomization(void);

void enableUSART1(void);
void USART1_SendChar(unsigned char c);
void USART1_SendString(const char* str);


// New function for direct RGB color control with PWM
void rgb_color(uint8_t red, uint8_t green, uint8_t blue);

#endif /* RGB_LED_H */

