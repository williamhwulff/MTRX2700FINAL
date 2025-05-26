#ifndef __L3GD20_H
#define __L3GD20_H

#include "stm32f3xx_hal.h"

// 用户需要将 hspi1 在 main.c 或外部声明
extern SPI_HandleTypeDef hspi1;

// GPIO 片选控制（PE3）
#define L3GD20_CS_PORT GPIOE
#define L3GD20_CS_PIN  GPIO_PIN_3

#define L3GD20_CS_LOW()   HAL_GPIO_WritePin(L3GD20_CS_PORT, L3GD20_CS_PIN, GPIO_PIN_RESET)
#define L3GD20_CS_HIGH()  HAL_GPIO_WritePin(L3GD20_CS_PORT, L3GD20_CS_PIN, GPIO_PIN_SET)

// 寄存器定义
#define L3GD20_REG_WHO_AM_I    0x0F
#define L3GD20_REG_CTRL1       0x20
#define L3GD20_REG_CTRL4       0x23
#define L3GD20_REG_OUT_X_L     0x28
#define L3GD20_REG_OUT_X_H     0x29
#define L3GD20_REG_OUT_Y_L     0x2A
#define L3GD20_REG_OUT_Y_H     0x2B
#define L3GD20_REG_OUT_Z_L     0x2C
#define L3GD20_REG_OUT_Z_H     0x2D

// 函数声明
void L3GD20_Init(void);
void L3GD20_Write(uint8_t reg, uint8_t data);
uint8_t L3GD20_Read(uint8_t reg);
void L3GD20_ReadXYZ(int16_t *x, int16_t *y, int16_t *z);
int16_t L3GD20_ReadAxis(uint8_t regL, uint8_t regH);

#endif  // __L3GD20_H
