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
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "font.h"
#include "oled.h"
#include "SR04.h"
#include "mpu6050.h"
#include "VL53L0X.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart3_tx;

uint8_t receivedata[50];
uint8_t transmitdata[20] = "系统开机！";
uint8_t mode2data[20] = "当前为模式2";
int mode;
int sum;

sr04_t sr04;
char message_sr04[20] = "";

MPU6050_t MPU6050;
char messagex[20] = "";
char messagey[20] = "";
char messagez[20] = "";

uint16_t vl53l0x_distance;
statInfo_t_VL53L0X distanceStr;
char message_vl[30] = "";
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
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
  if (huart == &huart2) {
    if (receivedata[0] == '2') {
      HAL_UART_Transmit_IT(&huart2, mode2data, strlen(mode2data));
    }


    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, receivedata, sizeof(receivedata));
    __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim == &htim4) {
    MPU6050_Read_All(&hi2c2, &MPU6050);
  }
}
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
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM1_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_I2C2_Init();
  MX_TIM4_Init();
  /* USER CODE BEGIN 2 */
  OLED_Init();

  HAL_UART_Transmit_IT(&huart2, transmitdata, strlen(transmitdata));
  HAL_UARTEx_ReceiveToIdle_DMA(&huart2, receivedata, sizeof(receivedata));
  __HAL_DMA_DISABLE_IT(&hdma_usart2_rx, DMA_IT_HT);

  //超声波测距SR04
  sr04.trig_port = GPIOA;
  sr04.trig_pin = GPIO_PIN_9;
  sr04.echo_htim = &htim1;
  sr04.echo_channel = TIM_CHANNEL_1;
  sr04_init(&sr04);

  //MPU6050
  while (MPU6050_Init(&hi2c2) == 1);

  // initVL53L0X(0, &hi2c2);

  HAL_TIM_Base_Start_IT(&htim4);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // sr04_trigger(&sr04);
    // HAL_Delay(100);
    // OLED_NewFrame();
    // sprintf(message_sr04, "distance: %lu", sr04.distance);
    // OLED_PrintString(0, 17, message_sr04, &font16x16, OLED_COLOR_NORMAL);
    // OLED_ShowFrame();

    //姿态传感器MPU
    sprintf(messagex, "roll: %.2f", MPU6050.KalmanAngleX);
    sprintf(messagey, "pitch: %.2f", MPU6050.KalmanAngleY);
    sprintf(messagez, "yaw: %.2f", MPU6050.AngleZ);
    OLED_NewFrame();
    OLED_PrintString(0, 0, messagex, &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 17, messagey, &font16x16, OLED_COLOR_NORMAL);
    OLED_PrintString(0, 33, messagez, &font16x16, OLED_COLOR_NORMAL);
    OLED_ShowFrame();
    sprintf(messagey, "pitch: %.2f \n", MPU6050.KalmanAngleY);
    HAL_UART_Transmit_DMA(&huart2,(uint8_t *) messagey, strlen(messagey));

    //TOF
    // vl53l0x_distance = readRangeSingleMillimeters(&distanceStr);
    // sprintf(message_vl, "distance: %d mm\r\n", vl53l0x_distance);
    // OLED_NewFrame();
    // OLED_PrintString(0, 0, message_vl, &font15x15, OLED_COLOR_NORMAL);
    // OLED_ShowFrame();
    // HAL_UART_Transmit_DMA(&huart2,(uint8_t *) message_vl, strlen(message_vl));
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
