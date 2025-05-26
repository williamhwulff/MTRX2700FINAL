#include "main.h"
#include "spi.h"
#include "gpio.h"
#include "stm32f3xx_hal_spi.h"

#define L3GD20_CS_LOW()   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET)
#define L3GD20_CS_HIGH()  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET)

void L3GD20_Write(uint8_t reg, uint8_t data) {
    uint8_t tx[2] = {reg & 0x7F, data};
    L3GD20_CS_LOW();
    HAL_SPI_Transmit(&hspi1, tx, 2, HAL_MAX_DELAY);
    L3GD20_CS_HIGH();
}

uint8_t L3GD20_Read(uint8_t reg) {
    uint8_t tx = reg | 0x80;
    uint8_t rx = 0;
    L3GD20_CS_LOW();
    HAL_SPI_Transmit(&hspi1, &tx, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, &rx, 1, HAL_MAX_DELAY);
    L3GD20_CS_HIGH();
    return rx;
}

void L3GD20_Init(void) {
    L3GD20_Write(0x20, 0x0F);  // CTRL_REG1: normal mode, all axes on, 95Hz
    L3GD20_Write(0x23, 0x10);  // CTRL_REG4: 500 dps full scale
}

int16_t L3GD20_ReadAxis(uint8_t regL, uint8_t regH) {
    uint8_t l = L3GD20_Read(regL);
    uint8_t h = L3GD20_Read(regH);
    return (int16_t)((h << 8) | l);
}

void L3GD20_ReadXYZ(int16_t* x, int16_t* y, int16_t* z) {
    *x = L3GD20_ReadAxis(0x28, 0x29);
    *y = L3GD20_ReadAxis(0x2A, 0x2B);
    *z = L3GD20_ReadAxis(0x2C, 0x2D);
}
