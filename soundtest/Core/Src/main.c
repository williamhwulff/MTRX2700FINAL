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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "serial.h"
#include <stdio.h>               // for snprintf
#include "serialise.h"
#include "servo.h"
#include "gpio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// timing window (±ms) around each beep:
#define TIMING_WINDOW_MS 500
// how many in-a-row to win:
#define GOAL_SCORE       5
#define LED_PORT       GPIOE
#define NUM_LEDS       8
#define BEEP_DEBOUNCE_MS 800
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

PCD_HandleTypeDef hpcd_USB_FS;

/* USER CODE BEGIN PV */
volatile uint32_t btn   = 0;   // timestamp of last button press
volatile uint32_t sound = 0;   // timestamp of last beep
volatile uint32_t streak = 0;  // current consecutive hits
volatile bool goal_reached = false;
static const uint16_t LED_PINS[NUM_LEDS] = {
    GPIO_PIN_8,
    GPIO_PIN_9,
    GPIO_PIN_10,
    GPIO_PIN_11,
	GPIO_PIN_12,
	GPIO_PIN_13,
	GPIO_PIN_14,
	GPIO_PIN_15
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USB_PCD_Init(void);
/* USER CODE BEGIN PFP */
static void my_exti_cb(uint16_t pin);
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

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_USB_PCD_Init();
  /* USER CODE BEGIN 2 */
  SerialInitialise(
     BAUD_115200,
     &USART1_PORT,
     NULL,     // no TX-complete callback
     NULL      // no RX callback
   );
  // === SERVO INIT ===
  enableClocks();           // turn on all GPIO banks
  setupGPIOPinsTim2();      // configure PA1–PA3, PA15 as TIM2 outputs
  setupTim2Pwm();           // set up TIM2 for 20 ms PWM
  servoAngle(2, 10);
  // 1) Configure PA2 as an input with pull-up
  GPIO_Init(GPIOA, GPIO_PIN_0, /*mode=*/0, /*pull=*/2);
  GPIO_Init(GPIOA, GPIO_PIN_2, /*mode=*/0, /*pull=*/2);

  // 2) Register your callback
  GPIO_EXTI_SetCallback(my_exti_cb);

  // 3) Enable EXTI on those pins
  GPIO_EXTI_Init(  GPIOA, GPIO_PIN_0, GPIO_EXTI_RISING);
  GPIO_EXTI_Init(  GPIOA, GPIO_PIN_2, GPIO_EXTI_FALLING);

  for (uint32_t i = 0; i < NUM_LEDS; i++) {
      GPIO_Init(
        LED_PORT,
        LED_PINS[i],
        /* mode = */ 1,    // 1 = general-purpose output
        /* pull = */ 0     // 0 = no pull
      );
  }

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      if (goal_reached) {
          goal_reached = false;          // clear the trigger
          servoAngle(2, 10);             // move servo in “safe” context
      }
  }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

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
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  PeriphClkInit.USBClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00201D2B;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_4BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USB Initialization Function
  * @param None
  * @retval None
  */
static void MX_USB_PCD_Init(void)
{

  /* USER CODE BEGIN USB_Init 0 */

  /* USER CODE END USB_Init 0 */

  /* USER CODE BEGIN USB_Init 1 */

  /* USER CODE END USB_Init 1 */
  hpcd_USB_FS.Instance = USB;
  hpcd_USB_FS.Init.dev_endpoints = 8;
  hpcd_USB_FS.Init.speed = PCD_SPEED_FULL;
  hpcd_USB_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
  hpcd_USB_FS.Init.low_power_enable = DISABLE;
  hpcd_USB_FS.Init.battery_charging_enable = DISABLE;
  if (HAL_PCD_Init(&hpcd_USB_FS) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USB_Init 2 */

  /* USER CODE END USB_Init 2 */

}

/* USER CODE BEGIN 4 */
/* helper to refresh the LEDs */
static void updateLeds(void)
{
    // clamp streak so we never over-run the array
    uint32_t s = (streak > NUM_LEDS ? NUM_LEDS : streak);

    for (uint32_t i = 0; i < NUM_LEDS; i++)
    {
        // use our gpio module’s write function
        GPIO_Write(
          LED_PORT,
          LED_PINS[i],
          (i < s) ? GPIO_PIN_SET : GPIO_PIN_RESET
        );
    }
}
static void my_exti_cb(uint16_t pin)
{
    uint32_t now = HAL_GetTick();
    static uint32_t last_beep_time = 0;

    if (pin == GPIO_PIN_2)  // sound detected (beep)
    {
        uint32_t now = HAL_GetTick();
        if (now - last_beep_time < BEEP_DEBOUNCE_MS)
            return;  // debounce: ignore if too soon after last beep

        last_beep_time = now;
        sound = now;

        Data d = { .beep_data = { .timestamp = now } };
        uint8_t packet[ sizeof(Header) + sizeof(BeepData) ];
        uint16_t len = pack_buffer(packet, BEEP_EVENT, &d);
        SerialOutputBuffer(packet, len, &USART1_PORT);
    }
    else if (pin == GPIO_PIN_0) // button pressed
    {
        btn = now;
        uint32_t delta = (btn > sound) ? (btn - sound) : (sound - btn);
        // === Transmit button press ===
        Data button_data = { .button_press_data = { .timestamp = now } };
        uint8_t packet[ sizeof(Header) + sizeof(ButtonPressData) ];
        uint16_t packet_len = pack_buffer(packet, BUTTON_PRESS, &button_data);
        SerialOutputBuffer(packet, packet_len, &USART1_PORT);

        if (delta <= TIMING_WINDOW_MS)
        {
            // Correct timing → increment streak
            streak++;

            // Serialize & send new streak
            Data d = { .streak_data = { .streak = streak } };
            uint8_t packet[ sizeof(Header) + sizeof(StreakData) ];
            uint16_t packet_len = pack_buffer(packet, STREAK_DATA, &d);
            SerialOutputBuffer(packet, packet_len, &USART1_PORT);
            // Optional: check for goal
            if (streak >= GOAL_SCORE)
            {
                goal_reached = true;
            }
        }
        else
        {
            // Incorrect timing (or no prior beep) → reset streak
            streak = 0;
            Data d = { .streak_data = { .streak = streak } };
            uint8_t packet[ sizeof(Header) + sizeof(StreakData) ];
            uint16_t packet_len = pack_buffer(packet, STREAK_DATA, &d);
            SerialOutputBuffer(packet, packet_len, &USART1_PORT);
        }

        updateLeds();  // redraw LEDs based on new streak
    }
}

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
  while (1)
  {
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
