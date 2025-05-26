// gpio.h
#ifndef GPIO_H
#define GPIO_H

#include "stm32f3xx.h"

// EXTI trigger types
#define GPIO_EXTI_RISING  1
#define GPIO_EXTI_FALLING 2
#define GPIO_EXTI_BOTH    3

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Initialize a GPIO pin (mode and pull-up/down)
 * @param  port GPIO port base address (e.g. GPIOA)
 * @param  pin  GPIO_PIN_x mask (e.g. GPIO_PIN_0)
 * @param  mode 2-bit mode field (0=input,1=output,2=AF,3=analog)
 * @param  pull 2-bit pull-up/pull-down (0=none,1=pull-up,2=pull-down)
 */
void GPIO_Init(GPIO_TypeDef *port, uint16_t pin, uint32_t mode, uint32_t pull);

/**
 * @brief  Write a GPIO output pin
 * @param  port  GPIO port
 * @param  pin   GPIO_PIN_x mask
 * @param  state GPIO_PinState (GPIO_PIN_SET or GPIO_PIN_RESET)
 */
void GPIO_Write(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state);

/**
 * @brief  Toggle a GPIO output pin
 * @param  port GPIO port
 * @param  pin  GPIO_PIN_x mask
 */
void GPIO_Toggle(GPIO_TypeDef *port, uint16_t pin);

/**
 * @brief  Register a callback for all EXTI events
 * @param  cb Function pointer to callback(uint16_t pin)
 */
void GPIO_EXTI_SetCallback(void (*cb)(uint16_t pin));

/**
 * @brief  Configure EXTI on a specific pin
 * @param  port GPIO port (for SYSCFG mapping)
 * @param  pin  GPIO_PIN_x mask
 * @param  trig GPIO_EXTI_RISING, GPIO_EXTI_FALLING or GPIO_EXTI_BOTH
 */
void GPIO_EXTI_Init(GPIO_TypeDef *port, uint16_t pin, uint32_t trig);

/**
 * @brief  Central EXTI IRQ handler to dispatch pending EXTI lines
 *         Call from all EXTIx IRQHandlers
 */
void GPIO_EXTI_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif // GPIO_H
