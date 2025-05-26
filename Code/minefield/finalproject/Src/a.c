#include "rgb_led.h"
/*
 * Hardware PWM Implementation for RGB LED Control on STM32F3 Discovery Board
 * File: software_ccr.c
 */

// Define the GPIO pins for RGB LED 1
#define RGB_RED_PIN     (6)   // PC6
#define RGB_GREEN_PIN   (7)   // PC7
#define RGB_BLUE_PIN    (8)   // PC8
#define RGB_PORT        GPIOC

// Define the GPIO pins for RGB LED 2
#define RGB2_RED_PIN    (0)   // PA0 - TIM2_CH1
#define RGB2_GREEN_PIN  (1)   // PA1 - TIM2_CH2
#define RGB2_BLUE_PIN   (2)   // PA2 - TIM2_CH3
#define RGB2_PORT       GPIOA


// Define the GPIO pins for RGB LED 3
#define RGB3_RED_PIN    (6)   // PB6 - TIM4_CH1
#define RGB3_GREEN_PIN  (7)   // PB7 - TIM4_CH2
#define RGB3_BLUE_PIN   (8)   // PB8 - TIM4_CH3
#define RGB3_PORT       GPIOB

// Define the GPIO pins for RGB LED 4
#define RGB4_RED_PIN    (8)   // PA8 - TIM1_CH1
#define RGB4_GREEN_PIN  (9)   // PA9 - TIM1_CH2
#define RGB4_BLUE_PIN   (10)  // PA10 - TIM1_CH3
#define RGB4_PORT       GPIOA


// Define the GPIO pin for limit switch 1
#define LIMIT_SWITCH_PIN (3)  // PA3
#define LIMIT_SWITCH_PORT GPIOA

// Define the GPIO pin for limit switch 2
#define LIMIT_SWITCH2_PIN (4)  // PA4
#define LIMIT_SWITCH2_PORT GPIOA

// Define the GPIO pin for limit switch 3
#define LIMIT_SWITCH3_PIN (5)  // PA5
#define LIMIT_SWITCH3_PORT GPIOA

// Define the GPIO pin for limit switch 4
#define LIMIT_SWITCH4_PIN (6)  // PA6
#define LIMIT_SWITCH4_PORT GPIOA

// Global variable to store the callback function
typedef void (*callback_t)(void);
callback_t timer_callback = NULL;
callback_t flash_timer_callback = NULL;

// Hardware CCR values for RGB1
uint8_t red_value = 0;    // Red value
uint8_t green_value = 0;  // Green value
uint8_t blue_value = 0;   // Blue value

// Hardware CCR values for RGB2
uint8_t red2_value = 0;    // Red value RGB2
uint8_t green2_value = 0;  // Green value RGB2
uint8_t blue2_value = 0;   // Blue value RGB2

// Hardware CCR values for RGB3
uint8_t red3_value = 0;    // Red value RGB3
uint8_t green3_value = 0;  // Green value RGB3
uint8_t blue3_value = 0;   // Blue value RGB3

// Hardware CCR values for RGB4
uint8_t red4_value = 0;    // Red value RGB4
uint8_t green4_value = 0;  // Green value RGB4
uint8_t blue4_value = 0;   // Blue value RGB4


// Color sequence counter
uint8_t current_state = 0;
uint8_t current_state2 = 0;
uint8_t current_state3 = 0;
uint8_t current_state4 = 0;


// Limit switch states
volatile uint8_t limit_switch_pressed = 0;
volatile uint8_t limit_switch2_pressed = 0;
volatile uint8_t flash_state = 0;
volatile uint8_t flash_active = 0;
volatile uint8_t flash2_state = 0;
volatile uint8_t flash2_active = 0;
volatile uint8_t limit_switch3_pressed = 0;
volatile uint8_t flash3_state = 0;
volatile uint8_t flash3_active = 0;
volatile uint8_t limit_switch4_pressed = 0;
volatile uint8_t flash4_state = 0;
volatile uint8_t flash4_active = 0;

// Store original LED colors when flashing starts
uint8_t original_red = 0;
uint8_t original_green = 0;
uint8_t original_blue = 0;

uint8_t original_red2 = 0;
uint8_t original_green2 = 0;
uint8_t original_blue2 = 0;

uint8_t original_red3 = 0;
uint8_t original_green3 = 0;
uint8_t original_blue3 = 0;

// Store original LED colors when flashing starts for RGB4
uint8_t original_red4 = 0;
uint8_t original_green4 = 0;
uint8_t original_blue4 = 0;

// Flash colors (bright white by default)
uint8_t flash_red = 255;
uint8_t flash_green = 255;
uint8_t flash_blue = 255;

uint8_t flash2_red = 255;
uint8_t flash2_green = 255;
uint8_t flash2_blue = 255;

uint8_t flash3_red = 255;
uint8_t flash3_green = 255;
uint8_t flash3_blue = 255;

// Flash color for RGB4
uint8_t flash4_red = 255;
uint8_t flash4_green = 255;
uint8_t flash4_blue = 255;


// Circular flash pattern variables
volatile uint8_t circular_flash_active = 0;
volatile uint8_t circular_flash_step = 0;  // 0-3 for RGB1-RGB4
volatile uint8_t circular_flash_state = 0; // ON/OFF state for current step

// Flash colors for circular pattern
uint8_t circular_colors[4][3] = {
    {255, 0, 0},    // RGB1: Bright Red
    {0, 255, 0},    // RGB2: Bright Green
    {0, 0, 255},    // RGB3: Bright Blue
    {255, 255, 0}   // RGB4: Bright Yellow
};


// Counter for rotating which LED displays yellow cycling
volatile uint8_t yellow_rotation_counter = 0;
volatile uint8_t current_yellow_led = 0; // 0=RGB1, 1=RGB2, 2=RGB3, 3=RGB4

// Function declarations for hardware PWM configuration
void configure_hardware_pwm(void);
void configure_hardware_pwm2(void);
void configure_hardware_pwm3(void);
void configure_hardware_pwm4(void);


void configure_limit_switch(void);
void configure_limit_switch2(void);
void configure_limit_switch3(void);
void configure_limit_switch4(void);

void configure_flash_timer(void);
void flash_led_callback(void);

void start_circular_flash(void);
void stop_circular_flash(void);
void circular_flash_callback(void);

// Enable the clocks for desired peripherals
void enable_clocks() {
    // Enable the clock for GPIO port C (for RGB LED 1)
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

    // Enable the clock for GPIO port A (for RGB LED 2 and limit switches)
    RCC->AHBENR |= RCC_AHBENR_GPIOAEN;

    // Enable the clock for GPIO port B (for RGB LED 3)
    RCC->AHBENR |= RCC_AHBENR_GPIOBEN;


    // Enable the peripheral clock for timer 16 (color cycling) - APB2 timer
    RCC->APB2ENR |= RCC_APB2ENR_TIM16EN;

    // Enable the peripheral clock for timer 3 (PWM generation for RGB1)
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;

    // Enable the peripheral clock for timer 2 (PWM generation for RGB2)
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    // Enable the peripheral clock for timer 4 (PWM generation for RGB3)
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    // Enable the peripheral clock for timer 1 (PWM generation for RGB4) - APB2 timer
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    // Enable the peripheral clock for timer 17 (LED flashing) - APB2 timer
    RCC->APB2ENR |= RCC_APB2ENR_TIM17EN;

    // Enable SYSCFG for EXTI
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // Add this line in enable_clocks() function:
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
}

// Initialize the RGB LED pins for timer output and limit switches as input
void initialise_board() {
    // Configure GPIO pins for RGB1 timer outputs (AF mode)
    // Clear the current mode bits
    RGB_PORT->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7 | GPIO_MODER_MODER8);

    // Set to Alternate Function mode (0b10)
    RGB_PORT->MODER |= (GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1 | GPIO_MODER_MODER8_1);

    // Set to AF2 (TIM3) for all three pins
    // First, clear the current AF
    RGB_PORT->AFR[0] &= ~(GPIO_AFRL_AFRL6 | GPIO_AFRL_AFRL7);
    RGB_PORT->AFR[1] &= ~(GPIO_AFRH_AFRH0); // For pin 8

    // Then set AF2 (0010)
    RGB_PORT->AFR[0] |= (0x2 << (4 * 6)) | (0x2 << (4 * 7)); // For pins 6 and 7
    RGB_PORT->AFR[1] |= (0x2 << (4 * 0)); // For pin 8 (as the 0th pin in the high register)

    // Set push-pull output type
    RGB_PORT->OTYPER &= ~((1 << RGB_RED_PIN) | (1 << RGB_GREEN_PIN) | (1 << RGB_BLUE_PIN));

    // Set high speed
    RGB_PORT->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR6 | GPIO_OSPEEDER_OSPEEDR7 | GPIO_OSPEEDER_OSPEEDR8);

    // No pull-up, pull-down
    RGB_PORT->PUPDR &= ~(GPIO_PUPDR_PUPDR6 | GPIO_PUPDR_PUPDR7 | GPIO_PUPDR_PUPDR8);

    // Configure GPIO pins for RGB2 timer outputs (AF mode)
    // Clear the current mode bits for PA0, PA1, PA2
    RGB2_PORT->MODER &= ~(GPIO_MODER_MODER0 | GPIO_MODER_MODER1 | GPIO_MODER_MODER2);

    // Set to Alternate Function mode (0b10)
    RGB2_PORT->MODER |= (GPIO_MODER_MODER0_1 | GPIO_MODER_MODER1_1 | GPIO_MODER_MODER2_1);

    // Set to AF1 (TIM2) for PA0, PA1, PA2
    // First, clear the current AF
    RGB2_PORT->AFR[0] &= ~(GPIO_AFRL_AFRL0 | GPIO_AFRL_AFRL1 | GPIO_AFRL_AFRL2);

    // Then set AF1 (0001)
    RGB2_PORT->AFR[0] |= (0x1 << (4 * 0)) | (0x1 << (4 * 1)) | (0x1 << (4 * 2));

    // Set push-pull output type
    RGB2_PORT->OTYPER &= ~((1 << RGB2_RED_PIN) | (1 << RGB2_GREEN_PIN) | (1 << RGB2_BLUE_PIN));

    // Set high speed
    RGB2_PORT->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR0 | GPIO_OSPEEDER_OSPEEDR1 | GPIO_OSPEEDER_OSPEEDR2);

    // No pull-up, pull-down
    RGB2_PORT->PUPDR &= ~(GPIO_PUPDR_PUPDR0 | GPIO_PUPDR_PUPDR1 | GPIO_PUPDR_PUPDR2);

    // Configure GPIO pins for RGB3 timer outputs (AF mode)
        // Clear the current mode bits for PB6, PB7, PB8
        RGB3_PORT->MODER &= ~(GPIO_MODER_MODER6 | GPIO_MODER_MODER7 | GPIO_MODER_MODER8);

        // Set to Alternate Function mode (0b10)
        RGB3_PORT->MODER |= (GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1 | GPIO_MODER_MODER8_1);

        // Set to AF2 (TIM4) for PB6, PB7, PB8
        // First, clear the current AF
        RGB3_PORT->AFR[0] &= ~(GPIO_AFRL_AFRL6 | GPIO_AFRL_AFRL7);
        RGB3_PORT->AFR[1] &= ~(GPIO_AFRH_AFRH0); // For pin 8

        // Then set AF2 (0010)
        RGB3_PORT->AFR[0] |= (0x2 << (4 * 6)) | (0x2 << (4 * 7)); // For pins 6 and 7
        RGB3_PORT->AFR[1] |= (0x2 << (4 * 0)); // For pin 8

        // Set push-pull output type
        RGB3_PORT->OTYPER &= ~((1 << RGB3_RED_PIN) | (1 << RGB3_GREEN_PIN) | (1 << RGB3_BLUE_PIN));

        // Set high speed
        RGB3_PORT->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR6 | GPIO_OSPEEDER_OSPEEDR7 | GPIO_OSPEEDER_OSPEEDR8);

        // No pull-up, pull-down
        RGB3_PORT->PUPDR &= ~(GPIO_PUPDR_PUPDR6 | GPIO_PUPDR_PUPDR7 | GPIO_PUPDR_PUPDR8);


        // Configure GPIO pins for RGB4 timer outputs (AF mode)
            // Clear the current mode bits for PA8, PA9, PA10
            RGB4_PORT->MODER &= ~(GPIO_MODER_MODER8 | GPIO_MODER_MODER9 | GPIO_MODER_MODER10);

            // Set to Alternate Function mode (0b10)
            RGB4_PORT->MODER |= (GPIO_MODER_MODER8_1 | GPIO_MODER_MODER9_1 | GPIO_MODER_MODER10_1);

            // Set to AF6 (TIM1) for PA8, PA9, PA10
            // Clear the current AF
            RGB4_PORT->AFR[1] &= ~(GPIO_AFRH_AFRH0 | GPIO_AFRH_AFRH1 | GPIO_AFRH_AFRH2); // PA8=AFR[1][0], PA9=AFR[1][1], PA10=AFR[1][2]

            // Then set AF6 (0110) for TIM1
            RGB4_PORT->AFR[1] |= (0x6 << (4 * 0)) | (0x6 << (4 * 1)) | (0x6 << (4 * 2)); // For pins 8, 9, 10

            // Set push-pull output type
            RGB4_PORT->OTYPER &= ~((1 << RGB4_RED_PIN) | (1 << RGB4_GREEN_PIN) | (1 << RGB4_BLUE_PIN));

            // Set high speed
            RGB4_PORT->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR8 | GPIO_OSPEEDER_OSPEEDR9 | GPIO_OSPEEDER_OSPEEDR10);

            // No pull-up, pull-down
            RGB4_PORT->PUPDR &= ~(GPIO_PUPDR_PUPDR8 | GPIO_PUPDR_PUPDR9 | GPIO_PUPDR_PUPDR10);

    // Configure hardware PWM
    configure_hardware_pwm();
    configure_hardware_pwm2();
    configure_hardware_pwm3();
    configure_hardware_pwm4();


    // Configure limit switches input
    configure_limit_switch();
    configure_limit_switch2();
    configure_limit_switch3();
    configure_limit_switch4();



    // Configure flash timer
    configure_flash_timer();

    enableUSART1();

}

// Configure TIM3 for hardware PWM (RGB1)
void configure_hardware_pwm(void) {
    // Reset timer configuration
    TIM3->CR1 = 0;

    // Set prescaler to 84-1 (counts from 0)
    // This divides the 84MHz APB clock to 1MHz
    TIM3->PSC = 84 - 1;

    // Set auto-reload value to 255 (8-bit resolution)
    TIM3->ARR = 255;

    // Enable auto-reload preload
    TIM3->CR1 |= TIM_CR1_ARPE;

    // Configure channels for PWM mode 1
    // PWM mode 1: Channel is active when TIMx_CNT < TIMx_CCRx

    // Configure Channel 1 (Red - PC6)
    TIM3->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM3->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // PWM mode 1 (0110)
    TIM3->CCMR1 |= TIM_CCMR1_OC1PE; // Enable preload for Channel 1

    // Configure Channel 2 (Green - PC7)
    TIM3->CCMR1 &= ~TIM_CCMR1_OC2M;
    TIM3->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2; // PWM mode 1 (0110)
    TIM3->CCMR1 |= TIM_CCMR1_OC2PE; // Enable preload for Channel 2

    // Configure Channel 3 (Blue - PC8)
    TIM3->CCMR2 &= ~TIM_CCMR2_OC3M;
    TIM3->CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2; // PWM mode 1 (0110)
    TIM3->CCMR2 |= TIM_CCMR2_OC3PE; // Enable preload for Channel 3

    // Enable all output channels
    TIM3->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E;

    // Set initial CCR values to 0 (all LEDs off)
    TIM3->CCR1 = 0; // Red
    TIM3->CCR2 = 0; // Green
    TIM3->CCR3 = 0; // Blue

    // Generate update event to load the registers
    TIM3->EGR |= TIM_EGR_UG;
}

// Configure TIM2 for hardware PWM (RGB2)
void configure_hardware_pwm2(void) {
    // Reset timer configuration
    TIM2->CR1 = 0;

    // TIM2 is typically on APB1 which might be different frequency
    // For STM32F3, APB1 is typically 36MHz or 72MHz depending on configuration
    // Set prescaler to get 1MHz timer clock
    // If APB1 = 36MHz: prescaler = 36-1 = 35
    // If APB1 = 72MHz: prescaler = 72-1 = 71
    // Let's try with 71 first (assuming 72MHz APB1)
    TIM2->PSC = 71; // This gives us ~1MHz timer clock

    // Set auto-reload value to 255 (8-bit resolution)
    // TIM2 is 32-bit, so this is fine
    TIM2->ARR = 255;

    // Enable auto-reload preload
    TIM2->CR1 |= TIM_CR1_ARPE;

    // Configure channels for PWM mode 1
    // PWM mode 1: Channel is active when TIMx_CNT < TIMx_CCRx

    // Configure Channel 1 (Red - PA0)
    TIM2->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM2->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // PWM mode 1 (0110)
    TIM2->CCMR1 |= TIM_CCMR1_OC1PE; // Enable preload for Channel 1

    // Configure Channel 2 (Green - PA1)
    TIM2->CCMR1 &= ~TIM_CCMR1_OC2M;
    TIM2->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2; // PWM mode 1 (0110)
    TIM2->CCMR1 |= TIM_CCMR1_OC2PE; // Enable preload for Channel 2

    // Configure Channel 3 (Blue - PA2)
    TIM2->CCMR2 &= ~TIM_CCMR2_OC3M;
    TIM2->CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2; // PWM mode 1 (0110)
    TIM2->CCMR2 |= TIM_CCMR2_OC3PE; // Enable preload for Channel 3

    // Enable all output channels
    TIM2->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E;

    // Set initial CCR values to test values (instead of 0)
    TIM2->CCR1 = 128; // Red - 50% duty cycle for testing
    TIM2->CCR2 = 128; // Green - 50% duty cycle for testing
    TIM2->CCR3 = 128; // Blue - 50% duty cycle for testing

    // Generate update event to load the registers
    TIM2->EGR |= TIM_EGR_UG;

    // Clear the update flag
    TIM2->SR &= ~TIM_SR_UIF;
}


// Configure TIM4 for hardware PWM (RGB3)
void configure_hardware_pwm3(void) {
    // Reset timer configuration
    TIM4->CR1 = 0;

    // TIM4 is on APB1, set prescaler to get 1MHz timer clock
    TIM4->PSC = 71; // Assuming 72MHz APB1

    // Set auto-reload value to 255 (8-bit resolution)
    TIM4->ARR = 255;

    // Enable auto-reload preload
    TIM4->CR1 |= TIM_CR1_ARPE;

    // Configure channels for PWM mode 1
    // Configure Channel 1 (Red - PB6)
    TIM4->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM4->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // PWM mode 1 (0110)
    TIM4->CCMR1 |= TIM_CCMR1_OC1PE; // Enable preload for Channel 1

    // Configure Channel 2 (Green - PB7)
    TIM4->CCMR1 &= ~TIM_CCMR1_OC2M;
    TIM4->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2; // PWM mode 1 (0110)
    TIM4->CCMR1 |= TIM_CCMR1_OC2PE; // Enable preload for Channel 2

    // Configure Channel 3 (Blue - PB8)
    TIM4->CCMR2 &= ~TIM_CCMR2_OC3M;
    TIM4->CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2; // PWM mode 1 (0110)
    TIM4->CCMR2 |= TIM_CCMR2_OC3PE; // Enable preload for Channel 3

    // Enable all output channels
    TIM4->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E;

    // Set initial CCR values to test values
    TIM4->CCR1 = 128; // Red - 50% duty cycle for testing
    TIM4->CCR2 = 128; // Green - 50% duty cycle for testing
    TIM4->CCR3 = 128; // Blue - 50% duty cycle for testing

    // Generate update event to load the registers
    TIM4->EGR |= TIM_EGR_UG;

    // Clear the update flag
    TIM4->SR &= ~TIM_SR_UIF;
}

// Configure TIM1 for hardware PWM (RGB4)
void configure_hardware_pwm4(void) {
    // Reset timer configuration
    TIM1->CR1 = 0;

    // TIM1 is on APB2, typically 72MHz
    // Set prescaler to get 1MHz timer clock: 72MHz / 72 = 1MHz
    TIM1->PSC = 71; // 72-1

    // Set auto-reload value to 255 (8-bit resolution)
    TIM1->ARR = 255;

    // Enable auto-reload preload
    TIM1->CR1 |= TIM_CR1_ARPE;

    // Configure channels for PWM mode 1
    // PWM mode 1: Channel is active when TIMx_CNT < TIMx_CCRx

    // Configure Channel 1 (Red - PA8)
    TIM1->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM1->CCMR1 |= TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2; // PWM mode 1 (0110)
    TIM1->CCMR1 |= TIM_CCMR1_OC1PE; // Enable preload for Channel 1

    // Configure Channel 2 (Green - PA9)
    TIM1->CCMR1 &= ~TIM_CCMR1_OC2M;
    TIM1->CCMR1 |= TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2; // PWM mode 1 (0110)
    TIM1->CCMR1 |= TIM_CCMR1_OC2PE; // Enable preload for Channel 2

    // Configure Channel 3 (Blue - PA10)
    TIM1->CCMR2 &= ~TIM_CCMR2_OC3M;
    TIM1->CCMR2 |= TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2; // PWM mode 1 (0110)
    TIM1->CCMR2 |= TIM_CCMR2_OC3PE; // Enable preload for Channel 3

    // Enable all output channels
    TIM1->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E;

    // CRITICAL: For advanced-control timers, enable the main output
    TIM1->BDTR |= TIM_BDTR_MOE; // Main Output Enable - required for TIM1

    // Set initial CCR values to test values
    TIM1->CCR1 = 128; // Red - 50% duty cycle for testing
    TIM1->CCR2 = 128; // Green - 50% duty cycle for testing
    TIM1->CCR3 = 128; // Blue - 50% duty cycle for testing

    // Generate update event to load the registers
    TIM1->EGR |= TIM_EGR_UG;

    // Clear the update flag
    TIM1->SR &= ~TIM_SR_UIF;
}


// Configure the limit switch 1 input pin and interrupt
void configure_limit_switch(void) {
    // Configure PA3 as input with pull-up
    LIMIT_SWITCH_PORT->MODER &= ~(GPIO_MODER_MODER3); // Input mode (00)
    LIMIT_SWITCH_PORT->PUPDR &= ~(GPIO_PUPDR_PUPDR3); // Clear existing setting
    LIMIT_SWITCH_PORT->PUPDR |= GPIO_PUPDR_PUPDR3_0;  // Pull-up (01)

    // Configure EXTI line 3 to be connected to GPIOA
    SYSCFG->EXTICR[0] &= ~(SYSCFG_EXTICR1_EXTI3); // Clear current setting
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI3_PA; // Set PA3 for EXTI3

    // Configure EXTI line 3
    EXTI->IMR |= EXTI_IMR_MR3;   // Enable interrupt on EXTI3
    EXTI->RTSR |= EXTI_RTSR_TR3; // Rising edge trigger (switch release)
    EXTI->FTSR |= EXTI_FTSR_TR3; // Falling edge trigger (switch press)

    // Enable EXTI3 interrupt in NVIC
    NVIC_SetPriority(EXTI3_IRQn, 1); // Higher priority than timers
    NVIC_EnableIRQ(EXTI3_IRQn);
}

// Configure the limit switch 2 input pin and interrupt
void configure_limit_switch2(void) {
    // Configure PA4 as input with pull-up
    LIMIT_SWITCH2_PORT->MODER &= ~(GPIO_MODER_MODER4); // Input mode (00)
    LIMIT_SWITCH2_PORT->PUPDR &= ~(GPIO_PUPDR_PUPDR4); // Clear existing setting
    LIMIT_SWITCH2_PORT->PUPDR |= GPIO_PUPDR_PUPDR4_0;  // Pull-up (01)

    // Configure EXTI line 4 to be connected to GPIOA
    SYSCFG->EXTICR[1] &= ~(SYSCFG_EXTICR2_EXTI4); // Clear current setting
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI4_PA; // Set PA4 for EXTI4

    // Configure EXTI line 4
    EXTI->IMR |= EXTI_IMR_MR4;   // Enable interrupt on EXTI4
    EXTI->RTSR |= EXTI_RTSR_TR4; // Rising edge trigger (switch release)
    EXTI->FTSR |= EXTI_FTSR_TR4; // Falling edge trigger (switch press)

    // Enable EXTI4 interrupt in NVIC
    NVIC_SetPriority(EXTI4_IRQn, 1); // Higher priority than timers
    NVIC_EnableIRQ(EXTI4_IRQn);
}


// Configure the limit switch 3 input pin and interrupt
void configure_limit_switch3(void) {
    // Configure PA5 as input with pull-up
    LIMIT_SWITCH3_PORT->MODER &= ~(GPIO_MODER_MODER5); // Input mode (00)
    LIMIT_SWITCH3_PORT->PUPDR &= ~(GPIO_PUPDR_PUPDR5); // Clear existing setting
    LIMIT_SWITCH3_PORT->PUPDR |= GPIO_PUPDR_PUPDR5_0;  // Pull-up (01)

    // Configure EXTI line 5 to be connected to GPIOA
    SYSCFG->EXTICR[1] &= ~(SYSCFG_EXTICR2_EXTI5); // Clear current setting
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI5_PA; // Set PA5 for EXTI5

    // Configure EXTI line 5
    EXTI->IMR |= EXTI_IMR_MR5;   // Enable interrupt on EXTI5
    EXTI->RTSR |= EXTI_RTSR_TR5; // Rising edge trigger (switch release)
    EXTI->FTSR |= EXTI_FTSR_TR5; // Falling edge trigger (switch press)

    // Enable EXTI9_5 interrupt in NVIC (EXTI5 uses EXTI9_5_IRQn)
    NVIC_SetPriority(EXTI9_5_IRQn, 1); // Higher priority than timers
    NVIC_EnableIRQ(EXTI9_5_IRQn);
}

// Configure the limit switch 4 input pin and interrupt
void configure_limit_switch4(void) {
    // Configure PA6 as input with pull-up
    LIMIT_SWITCH4_PORT->MODER &= ~(GPIO_MODER_MODER6); // Input mode (00)
    LIMIT_SWITCH4_PORT->PUPDR &= ~(GPIO_PUPDR_PUPDR6); // Clear existing setting
    LIMIT_SWITCH4_PORT->PUPDR |= GPIO_PUPDR_PUPDR6_0;  // Pull-up (01)

    // Configure EXTI line 6 to be connected to GPIOA
    SYSCFG->EXTICR[1] &= ~(SYSCFG_EXTICR2_EXTI6); // Clear current setting
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI6_PA; // Set PA6 for EXTI6

    // Configure EXTI line 6
    EXTI->IMR |= EXTI_IMR_MR6;   // Enable interrupt on EXTI6
    EXTI->RTSR |= EXTI_RTSR_TR6; // Rising edge trigger (switch release)
    EXTI->FTSR |= EXTI_FTSR_TR6; // Falling edge trigger (switch press)

    // Enable EXTI9_5 interrupt in NVIC (EXTI6 uses EXTI9_5_IRQn)
    NVIC_SetPriority(EXTI9_5_IRQn, 1); // Higher priority than timers
    NVIC_EnableIRQ(EXTI9_5_IRQn);
}


void configure_flash_timer(void) {
    // Reset timer configuration
    TIM17->CR1 = 0;

    // Set prescaler for 1kHz timer clock
    TIM17->PSC = 71999;

    // Set auto-reload value for slower flash interval
    TIM17->ARR = 199; // 200ms interval (twice as slow)
    // or
//    TIM17->ARR = 299; // 300ms interval (3x slower)
//    // or
//    TIM17->ARR = 499; // 500ms interval (5x slower)

    // Enable auto-reload preload
    TIM17->CR1 |= TIM_CR1_ARPE;

    // Generate update event to load the registers
    TIM17->EGR |= TIM_EGR_UG;

    // Clear the update flag
    TIM17->SR &= ~TIM_SR_UIF;

    // Enable update interrupt
    TIM17->DIER |= TIM_DIER_UIE;

    // Configure and enable TIM17 interrupt in NVIC
    NVIC_SetPriority(TIM1_TRG_COM_TIM17_IRQn, 3);
    NVIC_EnableIRQ(TIM1_TRG_COM_TIM17_IRQn);

    // Set the flash timer callback
    flash_timer_callback = flash_led_callback;
}

// Initialize timer for color cycling
void timer_init(uint32_t time_period_ms, callback_t cb) {
    // Disable the interrupts while configuring timer settings
    __disable_irq();

    // Store the callback function
    timer_callback = cb;

    // Disable interrupts for TIM16
    NVIC_DisableIRQ(TIM1_UP_TIM16_IRQn);

    // Clear any pending interrupt
    NVIC_ClearPendingIRQ(TIM1_UP_TIM16_IRQn);

    // Configure TIM16 (16-bit timer) for color cycling
    // Note: TIM16 is on APB2, which typically runs at higher frequency
    // Assuming APB2 = 72MHz, set prescaler for 1kHz timer clock (72MHz / 72000 = 1kHz)
    TIM16->PSC = 71999; // Adjust based on your actual APB2 clock frequency

    // Calculate the auto-reload value based on the desired time period
    // For 16-bit timer, maximum ARR value is 65535
    if (time_period_ms > 65535) {
        // If time period is too large, adjust prescaler
        // This is a simplified approach - you might need more sophisticated scaling
        time_period_ms = 65535;
    }
    TIM16->ARR = (time_period_ms - 1);

    // Generate an update event to load the new settings
    TIM16->EGR |= TIM_EGR_UG;

    // Clear the update flag that was set by the UG bit
    TIM16->SR &= ~TIM_SR_UIF;

    // Enable update interrupt for Timer 16
    TIM16->DIER |= TIM_DIER_UIE;

    // Configure and enable Timer 16 interrupt in NVIC
    NVIC_SetPriority(TIM1_UP_TIM16_IRQn, 2); // Lower priority
    NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);

    // Re-enable all interrupts
    __enable_irq();
}

/**
 * @brief Set RGB LED 1 color values
 * @param red Value for red LED (0-255)
 * @param green Value for green LED (0-255)
 * @param blue Value for blue LED (0-255)
 */
void set_rgb_ccr(uint8_t red, uint8_t green, uint8_t blue) {
    // Store values
    red_value = red;
    green_value = green;
    blue_value = blue;

    // Directly set hardware CCR registers
    TIM3->CCR1 = red;    // Red
    TIM3->CCR2 = green;  // Green
    TIM3->CCR3 = blue;   // Blue
}

/**
 * @brief Test function to verify TIM2 PWM is working
 */
void test_rgb2(void) {
    // Set RGB2 to bright white for testing
    set_rgb2_ccr(255, 255, 255);
}

/**
 * @brief Set RGB LED 2 color values
 * @param red Value for red LED (0-255)
 * @param green Value for green LED (0-255)
 * @param blue Value for blue LED (0-255)
 */
void set_rgb2_ccr(uint8_t red, uint8_t green, uint8_t blue) {
    // Store values
    red2_value = red;
    green2_value = green;
    blue2_value = blue;

    // Directly set hardware CCR registers for TIM2
    TIM2->CCR1 = red;    // Red - Channel 1
    TIM2->CCR2 = green;  // Green - Channel 2
    TIM2->CCR3 = blue;   // Blue - Channel 3
}


/**
 * @brief Test function to verify TIM4 PWM is working
 */
void test_rgb3(void) {
    // Set RGB3 to bright white for testing
    set_rgb3_ccr(255, 255, 255);
}

/**
 * @brief Set RGB LED 3 color values
 * @param red Value for red LED (0-255)
 * @param green Value for green LED (0-255)
 * @param blue Value for blue LED (0-255)
 */
void set_rgb3_ccr(uint8_t red, uint8_t green, uint8_t blue) {
    // Store values
    red3_value = red;
    green3_value = green;
    blue3_value = blue;

    // Directly set hardware CCR registers for TIM4
    TIM4->CCR1 = red;    // Red - Channel 1
    TIM4->CCR2 = green;  // Green - Channel 2
    TIM4->CCR3 = blue;   // Blue - Channel 3
}


/**
 * @brief Test function to verify TIM1 PWM is working
 */
void test_rgb4(void) {
    // Set RGB4 to bright white for testing (all 3 channels)
    set_rgb4_ccr(255, 255, 255);
}

/**
 * @brief Set RGB LED 4 color values
 * @param red Value for red LED (0-255)
 * @param green Value for green LED (0-255)
 * @param blue Value for blue LED (0-255)
 */
void set_rgb4_ccr(uint8_t red, uint8_t green, uint8_t blue) {
    // Store values
    red4_value = red;
    green4_value = green;
    blue4_value = blue;

    // Directly set hardware CCR registers for TIM1
    TIM1->CCR1 = red;    // Red - Channel 1
    TIM1->CCR2 = green;  // Green - Channel 2
    TIM1->CCR3 = blue;   // Blue - Channel 3
}


// Set RGB LED 1 color based on current demonstration state
void set_rgb_color(void) {
    // Only change colors if not in flashing mode
    if (!flash_active) {
        // Check if RGB1 should display yellow cycling
        if (current_yellow_led == 0) {
            // Yellow cycling sequence
            switch (current_state) {
                case 0:
                    set_rgb_ccr(200, 230, 0);
                    break;
                case 1:
                    set_rgb_ccr(190, 200, 0);
                    break;
                case 2:
                    set_rgb_ccr(200, 255, 0);
                    break;
                case 3:
                    set_rgb_ccr(210, 240, 0);
                    break;
                case 4:
                    set_rgb_ccr(200, 240, 0);
                    break;
                case 5:
                    set_rgb_ccr(180, 200, 10);
                    break;
            }
            // Move to the next state
            current_state = (current_state + 1) % 6;
        } else {
            // Red cycling sequence
            switch (current_state) {
                case 0:
                    set_rgb_ccr(120, 50, 50);
                    break;
                case 1:
                    set_rgb_ccr(255, 255, 255);
                    break;
            }
            // Move to the next state
            current_state = (current_state + 1) % 2;
        }

        // Store the original colors (for when flashing starts)
        original_red = red_value;
        original_green = green_value;
        original_blue = blue_value;
    }
}

void set_rgb2_color(void) {
    // Only change colors if not in flashing mode
    if (!flash2_active) {
        // Check if RGB2 should display yellow cycling
        if (current_yellow_led == 1) {
            // Yellow cycling sequence
            switch (current_state2) {
                case 0:
                    set_rgb2_ccr(200, 230, 0);
                    break;
                case 1:
                    set_rgb2_ccr(190, 200, 0);
                    break;
                case 2:
                    set_rgb2_ccr(200, 255, 0);
                    break;
                case 3:
                    set_rgb2_ccr(210, 240, 0);
                    break;
                case 4:
                    set_rgb2_ccr(200, 240, 0);
                    break;
                case 5:
                    set_rgb2_ccr(180, 200, 10);
                    break;
            }
            // Move to the next state
            current_state2 = (current_state2 + 1) % 6;
        } else {
            // Red cycling sequence
            switch (current_state2) {
                case 0:
                    set_rgb2_ccr(120, 50, 50);
                    break;
                case 1:
                    set_rgb2_ccr(255, 255, 255);
                    break;
            }
            // Move to the next state
            current_state2 = (current_state2 + 1) % 2;
        }

        // Store the original colors (for when flashing starts)
        original_red2 = red2_value;
        original_green2 = green2_value;
        original_blue2 = blue2_value;
    }
}

void set_rgb3_color(void) {
    // Only change colors if not in flashing mode
    if (!flash3_active) {
        // Check if RGB3 should display yellow cycling
        if (current_yellow_led == 2) {
            // Yellow cycling sequence
            switch (current_state3) {
                case 0:
                    set_rgb3_ccr(200, 230, 0);
                    break;
                case 1:
                    set_rgb3_ccr(190, 200, 0);
                    break;
                case 2:
                    set_rgb3_ccr(200, 255, 0);
                    break;
                case 3:
                    set_rgb3_ccr(210, 240, 0);
                    break;
                case 4:
                    set_rgb3_ccr(200, 240, 0);
                    break;
                case 5:
                    set_rgb3_ccr(180, 200, 10);
                    break;
            }
            // Move to the next state
            current_state3 = (current_state3 + 1) % 6;
        } else {
            // Red cycling sequence
            switch (current_state3) {
                case 0:
                    set_rgb3_ccr(120, 50, 50);
                    break;
                case 1:
                    set_rgb3_ccr(255, 255, 255);
                    break;
            }
            // Move to the next state
            current_state3 = (current_state3 + 1) % 2;
        }

        // Store the original colors (for when flashing starts)
        original_red3 = red3_value;
        original_green3 = green3_value;
        original_blue3 = blue3_value;
    }
}

void set_rgb4_color(void) {
    if (!flash4_active) {
        // Check if RGB4 should display yellow cycling
        if (current_yellow_led == 3) {
            // Yellow cycling sequence
            switch (current_state4) {
                case 0:
                    set_rgb4_ccr(200, 230, 0);
                    break;
                case 1:
                    set_rgb4_ccr(190, 200, 0);
                    break;
                case 2:
                    set_rgb4_ccr(200, 255, 0);
                    break;
                case 3:
                    set_rgb4_ccr(210, 240, 0);
                    break;
                case 4:
                    set_rgb4_ccr(200, 240, 0);
                    break;
                case 5:
                    set_rgb4_ccr(180, 200, 10);
                    break;
            }
            // Move to the next state
            current_state4 = (current_state4 + 1) % 6;
        } else {
            // Red cycling sequence
            switch (current_state4) {
                case 0:
                    set_rgb4_ccr(120, 50, 50);
                    break;
                case 1:
                    set_rgb4_ccr(255, 255, 255);
                    break;
            }
            // Move to the next state
            current_state4 = (current_state4 + 1) % 2;
        }

        // Store original colors
        original_red4 = red4_value;
        original_green4 = green4_value;
        original_blue4 = blue4_value;
    }
}


// Flash LED callback function
void flash_led_callback(void) {
    // Check if circular flash is active first
    if (circular_flash_active) {
        circular_flash_callback();
        return; // Don't process individual flashing if circular is active
    }

    // Original individual flashing code
    // Handle RGB1 flashing
    if (flash_active) {
        if (flash_state) {
            // Flash ON state - show flash color
            set_rgb_ccr(flash_red, flash_green, flash_blue);
        } else {
            // Flash OFF state - turn LEDs off
            set_rgb_ccr(0, 0, 0);
        }
        // Toggle flash state
        flash_state = !flash_state;
    }

    // Handle RGB2 flashing
    if (flash2_active) {
        if (flash2_state) {
            // Flash ON state - show flash color
            set_rgb2_ccr(flash2_red, flash2_green, flash2_blue);
        } else {
            // Flash OFF state - turn LEDs off
            set_rgb2_ccr(0, 0, 0);
        }
        // Toggle flash state
        flash2_state = !flash2_state;
    }

    // Handle RGB3 flashing
    if (flash3_active) {
        if (flash3_state) {
            // Flash ON state - show flash color
            set_rgb3_ccr(flash3_red, flash3_green, flash3_blue);
        } else {
            // Flash OFF state - turn LEDs off
            set_rgb3_ccr(0, 0, 0);
        }
        // Toggle flash state
        flash3_state = !flash3_state;
    }

    // Handle RGB4 flashing
    if (flash4_active) {
        if (flash4_state) {
            // Flash ON state - show flash color
            set_rgb4_ccr(flash4_red, flash4_green, flash4_blue);
        } else {
            // Flash OFF state - turn LEDs off
            set_rgb4_ccr(0, 0, 0);
        }
        // Toggle flash state
        flash4_state = !flash4_state;
    }
}


// Enable all timers to start counting
void enable_timer(void) {
    // Enable the Timer 16 counter (color cycling)
    TIM16->CR1 |= TIM_CR1_CEN;

    // Enable the Timer 3 counter (PWM generation for RGB1)
    TIM3->CR1 |= TIM_CR1_CEN;

    // Enable the Timer 2 counter (PWM generation for RGB2)
    TIM2->CR1 |= TIM_CR1_CEN;

    // Enable the Timer 4 counter (PWM generation for RGB3)
    TIM4->CR1 |= TIM_CR1_CEN;

    // Enable the Timer 1 counter (PWM generation for RGB4)
    TIM1->CR1 |= TIM_CR1_CEN;
}

// Set the flash color for RGB1
void set_flash_color(uint8_t red, uint8_t green, uint8_t blue) {
    flash_red = red;
    flash_green = green;
    flash_blue = blue;
}

// Set the flash color for RGB2
void set_flash2_color(uint8_t red, uint8_t green, uint8_t blue) {
    flash2_red = red;
    flash2_green = green;
    flash2_blue = blue;
}

// Set the flash color for RGB3
void set_flash3_color(uint8_t red, uint8_t green, uint8_t blue) {
    flash3_red = red;
    flash3_green = green;
    flash3_blue = blue;
}


// Set the flash color for RGB4
void set_flash4_color(uint8_t red, uint8_t green, uint8_t blue) {
    flash4_red = red;
    flash4_green = green;
    flash4_blue = blue;
}

// Start flashing RGB1 LEDs
void start_flashing(void) {
    if (!flash_active) {
        flash_active = 1;
        flash_state = 1; // Start with flash ON

        // Store current colors
        original_red = red_value;
        original_green = green_value;
        original_blue = blue_value;

        // Enable the flash timer if not already running
        if (!flash2_active) {
            TIM17->CR1 |= TIM_CR1_CEN;
        }
    }
}

// Stop flashing RGB1 LEDs
void stop_flashing(void) {
    if (flash_active) {
        flash_active = 0;

        // Restore original colors
        set_rgb_ccr(original_red, original_green, original_blue);

        // Disable the flash timer only if RGB2 is not flashing
        if (!flash2_active) {
            TIM17->CR1 &= ~TIM_CR1_CEN;
        }
    }
}

// Start flashing RGB2 LEDs
void start_flashing2(void) {
    if (!flash2_active) {
        flash2_active = 1;
        flash2_state = 1; // Start with flash ON

        // Store current colors
        original_red2 = red2_value;
        original_green2 = green2_value;
        original_blue2 = blue2_value;

        // Enable the flash timer if not already running
        if (!flash_active) {
            TIM17->CR1 |= TIM_CR1_CEN;
        }
    }
}

// Stop flashing RGB2 LEDs
void stop_flashing2(void) {
    if (flash2_active) {
        flash2_active = 0;

        // Restore original colors
        set_rgb2_ccr(original_red2, original_green2, original_blue2);

        // Disable the flash timer only if RGB1 is not flashing
        if (!flash_active) {
            TIM17->CR1 &= ~TIM_CR1_CEN;
        }
    }
}

// Start flashing RGB3 LEDs
void start_flashing3(void) {
    if (!flash3_active) {
        flash3_active = 1;
        flash3_state = 1; // Start with flash ON

        // Store current colors
        original_red3 = red3_value;
        original_green3 = green3_value;
        original_blue3 = blue3_value;

        // Enable the flash timer if not already running
        if (!flash_active && !flash2_active) {
            TIM17->CR1 |= TIM_CR1_CEN;
        }
    }
}

// Stop flashing RGB3 LEDs
void stop_flashing3(void) {
    if (flash3_active) {
        flash3_active = 0;

        // Restore original colors
        set_rgb3_ccr(original_red3, original_green3, original_blue3);

        // Disable the flash timer only if no other LEDs are flashing
        if (!flash_active && !flash2_active) {
            TIM17->CR1 &= ~TIM_CR1_CEN;
        }
    }
}


// Start flashing RGB4 LEDs
void start_flashing4(void) {
    if (!flash4_active) {
        flash4_active = 1;
        flash4_state = 1; // Start with flash ON

        // Store current colors
        original_red4 = red4_value;
        original_green4 = green4_value;
        original_blue4 = blue4_value;

        // Enable the flash timer if not already running
        if (!flash_active && !flash2_active && !flash3_active) {
            TIM17->CR1 |= TIM_CR1_CEN;
        }
    }
}

// Stop flashing RGB4 LEDs
void stop_flashing4(void) {
    if (flash4_active) {
        flash4_active = 0;

        // Restore original colors
        set_rgb4_ccr(original_red4, original_green4, original_blue4);

        // Disable the flash timer only if no other LEDs are flashing
        if (!flash_active && !flash2_active && !flash3_active) {
            TIM17->CR1 &= ~TIM_CR1_CEN;
        }
    }
}


// Timer 16 Interrupt Service Routine - for color cycling
void TIM1_UP_TIM16_IRQHandler(void) {
    if (TIM16->SR & TIM_SR_UIF) {
        // Clear the update interrupt flag
        TIM16->SR &= ~TIM_SR_UIF;

        // Increment the yellow rotation counter
        yellow_rotation_counter++;

        // Check if 50 cycles have passed
        if (yellow_rotation_counter >= 10) {
            yellow_rotation_counter = 0; // Reset counter
            current_yellow_led = (current_yellow_led + 1) % 4; // Rotate to next LED
        }

        // Call the callback function if it's not NULL
        if (timer_callback != NULL) {
            timer_callback();
        }

        // Cycle all LED colors
        set_rgb2_color();
        set_rgb3_color();
        set_rgb4_color();
    }
}

// Timer 17 Interrupt Service Routine - for LED flashing
void TIM1_TRG_COM_TIM17_IRQHandler(void) {
    if (TIM17->SR & TIM_SR_UIF) {
        // Clear the update interrupt flag
        TIM17->SR &= ~TIM_SR_UIF;

        // Call the flash callback function if it's not NULL
        if (flash_timer_callback != NULL) {
            flash_timer_callback();
        }
    }
}
void EXTI3_IRQHandler(void) {
    // Check if EXTI3 was triggered
    if (EXTI->PR & EXTI_PR_PR3) {
        // Clear the pending bit
        EXTI->PR = EXTI_PR_PR3;

        // Read the current state of the limit switch
        uint8_t switch_state = (LIMIT_SWITCH_PORT->IDR & (1 << LIMIT_SWITCH_PIN)) ? 0 : 1;

        // Update limit switch state
        limit_switch_pressed = switch_state;

        // If limit switch is pressed
        if (limit_switch_pressed) {
            // Check if this button corresponds to current yellow LED
            if (current_yellow_led == 0) {
                USART1_SendString("GOLD");
            } else {
                USART1_SendString("KABOOM");
            }
            start_circular_flash();
        } else {
            // When released, stop circular flashing
            stop_circular_flash();
        }
    }
}
void EXTI4_IRQHandler(void) {
    // Check if EXTI4 was triggered
    if (EXTI->PR & EXTI_PR_PR4) {
        // Clear the pending bit
        EXTI->PR = EXTI_PR_PR4;

        // Read the current state of the limit switch 2
        uint8_t switch_state = (LIMIT_SWITCH2_PORT->IDR & (1 << LIMIT_SWITCH2_PIN)) ? 0 : 1;

        // Update limit switch 2 state
        limit_switch2_pressed = switch_state;

        // If limit switch 2 is pressed
        if (limit_switch2_pressed) {
            // Check if this button corresponds to current yellow LED
            if (current_yellow_led == 1) {
                USART1_SendString("GOLD");
            } else {
                USART1_SendString("KABOOM");
            }
            start_circular_flash();
        } else {
            // When released, stop circular flashing
            stop_circular_flash();
        }
    }
}

void EXTI9_5_IRQHandler(void) {
    // Check if EXTI5 was triggered (limit switch 3)
    if (EXTI->PR & EXTI_PR_PR5) {
        // Clear the pending bit
        EXTI->PR = EXTI_PR_PR5;

        // Read the current state of the limit switch 3
        uint8_t switch_state = (LIMIT_SWITCH3_PORT->IDR & (1 << LIMIT_SWITCH3_PIN)) ? 0 : 1;

        // Update limit switch 3 state
        limit_switch3_pressed = switch_state;

        // If limit switch 3 is pressed
        if (limit_switch3_pressed) {
            // Check if this button corresponds to current yellow LED
            if (current_yellow_led == 2) {
                USART1_SendString("GOLD");
            } else {
                USART1_SendString("KABOOM");
            }
            start_circular_flash();
        } else {
            // When released, stop circular flashing
            stop_circular_flash();
        }
    }

    // Check if EXTI6 was triggered (limit switch 4)
    if (EXTI->PR & EXTI_PR_PR6) {
        // Clear the pending bit
        EXTI->PR = EXTI_PR_PR6;

        // Read the current state of the limit switch 4
        uint8_t switch_state = (LIMIT_SWITCH4_PORT->IDR & (1 << LIMIT_SWITCH4_PIN)) ? 0 : 1;

        // Update limit switch 4 state
        limit_switch4_pressed = switch_state;

        // If limit switch 4 is pressed
        if (limit_switch4_pressed) {
            // Check if this button corresponds to current yellow LED
            if (current_yellow_led == 3) {
                USART1_SendString("GOLD");
            } else {
                USART1_SendString("KABOOM");
            }
            start_circular_flash();
        } else {
            // When released, stop circular flashing
            stop_circular_flash();
        }
    }
}


// Start the circular flash pattern
void start_circular_flash(void) {
    if (!circular_flash_active) {
        circular_flash_active = 1;
        circular_flash_step = 0;  // Start with RGB1
        circular_flash_state = 1; // Start with flash ON

        // STOP the color cycling timer to prevent interference
        TIM16->CR1 &= ~TIM_CR1_CEN;

        // Turn off all individual flashing
        flash_active = 0;
        flash2_active = 0;
        flash3_active = 0;
        flash4_active = 0;

        // Store current colors before turning off
        original_red = red_value;
        original_green = green_value;
        original_blue = blue_value;
        original_red2 = red2_value;
        original_green2 = green2_value;
        original_blue2 = blue2_value;
        original_red3 = red3_value;
        original_green3 = green3_value;
        original_blue3 = blue3_value;
        original_red4 = red4_value;
        original_green4 = green4_value;
        original_blue4 = blue4_value;

        // Set all LEDs to off initially
        set_rgb_ccr(0, 0, 0);
        set_rgb2_ccr(0, 0, 0);
        set_rgb3_ccr(0, 0, 0);
        set_rgb4_ccr(0, 0, 0);

        // Enable the flash timer
        TIM17->CR1 |= TIM_CR1_CEN;
    }
}

// Stop the circular flash pattern
void stop_circular_flash(void) {
    if (circular_flash_active) {
        circular_flash_active = 0;

        // Disable the flash timer first
        TIM17->CR1 &= ~TIM_CR1_CEN;

        // Restore all LEDs to their normal cycling colors
        set_rgb_ccr(original_red, original_green, original_blue);
        set_rgb2_ccr(original_red2, original_green2, original_blue2);
        set_rgb3_ccr(original_red3, original_green3, original_blue3);
        set_rgb4_ccr(original_red4, original_green4, original_blue4);

        // RESTART the color cycling timer
        TIM16->CR1 |= TIM_CR1_CEN;
    }
}

// Circular flash callback function
void circular_flash_callback(void) {
    if (circular_flash_active) {
        // Always turn off all LEDs first
        set_rgb_ccr(0, 0, 0);
        set_rgb2_ccr(0, 0, 0);
        set_rgb3_ccr(0, 0, 0);
        set_rgb4_ccr(0, 0, 0);

        if (circular_flash_state) {
            // Flash ON state - light up ONLY the current LED in sequence
            switch (circular_flash_step) {
                case 0: // Only RGB1
                    set_rgb_ccr(circular_colors[0][0], circular_colors[0][1], circular_colors[0][2]);
                    break;
                case 1: // Only RGB2
                    set_rgb2_ccr(circular_colors[1][0], circular_colors[1][1], circular_colors[1][2]);
                    break;
                case 2: // Only RGB3
                    set_rgb3_ccr(circular_colors[2][0], circular_colors[2][1], circular_colors[2][2]);
                    break;
                case 3: // Only RGB4
                    set_rgb4_ccr(circular_colors[3][0], circular_colors[3][1], circular_colors[3][2]);
                    break;
            }

            // Move to next LED in sequence
            circular_flash_step = (circular_flash_step + 1) % 4;
        }
        // If circular_flash_state is 0 (OFF), all LEDs stay off (already set above)

        // Toggle flash state
        circular_flash_state = !circular_flash_state;
    }
}

void enableUSART1()
{
    // Enable GPIO C clock (already enabled for RGB1, but ensure it's there)
    RCC->AHBENR |= RCC_AHBENR_GPIOCEN;

    // Set GPIO C pins 4&5 to use UART as alternate function
    // Save current MODER settings for other pins
    uint32_t temp_moder = GPIOC->MODER;
    temp_moder &= ~(GPIO_MODER_MODER4 | GPIO_MODER_MODER5); // Clear PC4, PC5
    temp_moder |= (GPIO_MODER_MODER4_1 | GPIO_MODER_MODER5_1); // Set to AF mode
    GPIOC->MODER = temp_moder;

    // Set alternate function for PC4 (TX) and PC5 (RX)
    GPIOC->AFR[0] &= ~(GPIO_AFRL_AFRL4 | GPIO_AFRL_AFRL5); // Clear
    GPIOC->AFR[0] |= (0x7 << (4 * 4)) | (0x7 << (4 * 5)); // AF7 for USART1

    // Set high speed
    GPIOC->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR4 | GPIO_OSPEEDER_OSPEEDR5);

    // Set the baud rate and ready USART 1 for transmit
    USART1->BRR = BAUDRATE; // Baud rate = 115200
    USART1->CR1 |= USART_CR1_TE; // Enable transmitter
    USART1->CR1 |= USART_CR1_UE; // Enable USART
}

// Function to send a single character
void USART1_SendChar(unsigned char c)
{
    // Wait until the transmit data register is empty
    while(!(USART1->ISR & USART_ISR_TXE));

    // Write the character to the transmit data register
    USART1->TDR = c;
}

// Function to send a string
void USART1_SendString(const char* str)
{
    while(*str)
    {
        USART1_SendChar(*str++);
    }

    // Send carriage return and line feed for proper line ending in terminal
    USART1_SendChar('\r');
    USART1_SendChar('\n');
}
