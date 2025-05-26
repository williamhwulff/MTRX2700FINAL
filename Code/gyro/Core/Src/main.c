/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb.h"
#include "gpio.h"
#include <stdlib.h>

#define L3GD20_CS_LOW()   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET)
#define L3GD20_CS_HIGH()  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_SET)

extern SPI_HandleTypeDef hspi1;

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

	HAL_Init();
	    SystemClock_Config();
	    MX_GPIO_Init();
	    MX_SPI1_Init();
	    MX_TIM2_Init();

	    L3GD20_Init();

	    // Start PWM
	    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
	    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);

	    // Bias calibration
	    float gyro_bias_x = 0, gyro_bias_y = 0;
	    for (int i = 0; i < 100; i++) {
	        gyro_bias_x += L3GD20_ReadAxis(0x28, 0x29);
	        gyro_bias_y += L3GD20_ReadAxis(0x2A, 0x2B);
	        HAL_Delay(5);
	    }
	    gyro_bias_x /= 100.0f;
	    gyro_bias_y /= 100.0f;

	    float angle_x = 0.0f, angle_y = 0.0f;
	    uint32_t last_time = HAL_GetTick();
	    uint16_t vertical_PWM = 1500;
	    uint16_t horizontal_PWM = 1500;

  while (1)
  {
	  int16_t roll_rate_raw = L3GD20_ReadAxis(0x28, 0x29);   // X axis
	  int16_t pitch_rate_raw = L3GD20_ReadAxis(0x2A, 0x2B);  // Y axis

	  uint32_t now = HAL_GetTick();
	  float delta_time = (now - last_time) / 1000.0f;
	  last_time = now;

	  float servo_gain = 0.25f; // Reduce sensitivity by half
	  float x_rate_dps = ((float)roll_rate_raw - gyro_bias_x) * 0.00875f * servo_gain;
	  float y_rate_dps = ((float)pitch_rate_raw - gyro_bias_y) * 0.00875f * servo_gain;

	  angle_x += x_rate_dps * delta_time;
	  angle_y += y_rate_dps * delta_time;

	  if (angle_x > 10) angle_x = 10;
	  if (angle_x < -10) angle_x = -10;
	  if (angle_y > 10) angle_y = 10;
	  if (angle_y < -10) angle_y = -10;

	  vertical_PWM = 1100 + (int)(angle_y * 20);
	  horizontal_PWM = 1000 + (int)(angle_x * 20);

	  // Vertical is closest to the ptu
	  // The
	  if (vertical_PWM > 1180) vertical_PWM = 1180;
	  //turn arm down
	  if (vertical_PWM < 1020) vertical_PWM = 1020;
	  // Horizontal the top servo
	  // Upper bound twists into box
	  if (horizontal_PWM > 1048) horizontal_PWM = 1080;
	  if (horizontal_PWM < 920) horizontal_PWM = 920;

	  TIM2->CCR1 = vertical_PWM;
	  TIM2->CCR2 = horizontal_PWM;

	  HAL_Delay(10);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  PeriphClkInit.USBClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_8, GPIO_PIN_SET);
  while (1)
  {
      HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_8);
      HAL_Delay(500);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
