// gpio.c
#include "gpio.h"

static void enable_clk(GPIO_TypeDef *p) {
    if      (p == GPIOA) RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
    else if (p == GPIOB) RCC->AHBENR |= RCC_AHBENR_GPIOBEN;
    else if (p == GPIOC) RCC->AHBENR |= RCC_AHBENR_GPIOCEN;
    else if (p == GPIOD) RCC->AHBENR |= RCC_AHBENR_GPIODEN;
    else if (p == GPIOE) RCC->AHBENR |= RCC_AHBENR_GPIOEEN;
}

void GPIO_Init(GPIO_TypeDef *port, uint16_t pin, uint32_t mode, uint32_t pull) {
    enable_clk(port);
    uint32_t pos = __builtin_ctz(pin);
    port->MODER = (port->MODER & ~(3u << (pos*2)))
                |  (mode   << (pos*2));
    port->PUPDR = (port->PUPDR & ~(3u << (pos*2)))
                |  (pull   << (pos*2));
}

void GPIO_Write(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState state) {
    if (state == GPIO_PIN_SET) port->BSRR = pin;
    else                       port->BRR  = pin;
}

void GPIO_Toggle(GPIO_TypeDef *port, uint16_t pin) {
    if (port->ODR & pin) port->BRR  = pin;
    else                 port->BSRR = pin;
}

// EXTI support
static void (*exti_cb)(uint16_t) = 0;

void GPIO_EXTI_SetCallback(void (*cb)(uint16_t pin)) {
    exti_cb = cb;
}

void GPIO_EXTI_Init(GPIO_TypeDef *port, uint16_t pin, uint32_t trig) {
    uint32_t line  = __builtin_ctz(pin);
    RCC->APB2ENR  |= RCC_APB2ENR_SYSCFGEN;
    uint32_t idx    = line >> 2;
    uint32_t shift  = (line & 3) * 4;
    uint32_t src    = (port==GPIOA?0:
                       port==GPIOB?1:
                       port==GPIOC?2:
                       port==GPIOD?3:4);
    SYSCFG->EXTICR[idx] = (SYSCFG->EXTICR[idx] & ~(0xFu << shift))
                        |  (src << shift);
    if (trig & GPIO_EXTI_RISING)  EXTI->RTSR |= (1u << line);
    if (trig & GPIO_EXTI_FALLING) EXTI->FTSR |= (1u << line);
    EXTI->IMR |= (1u << line);

    // enable all EXTI IRQs once
    NVIC_EnableIRQ(EXTI0_IRQn);        // for EXTI line 0 (PA0)
    NVIC_EnableIRQ(EXTI2_TSC_IRQn);    // for EXTI line 2 (PA2, shared w/ TSC)
}
void GPIO_EXTI_IRQHandler(void) {
    for (uint32_t l = 0; l < 16; l++) {
        uint32_t mask = (1u << l);
        if (EXTI->PR & mask) {
            EXTI->PR = mask;
            if (exti_cb) exti_cb(mask);
        }
    }
}
