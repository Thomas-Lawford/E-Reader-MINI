/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u0xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);
void SystemClock_Config_LowPower(void);
void SystemClock_Config_Normal(void);
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define FWD_Pin GPIO_PIN_0
#define FWD_GPIO_Port GPIOA
#define BCK_Pin GPIO_PIN_1
#define BCK_GPIO_Port GPIOA
#define SD_CS_Pin GPIO_PIN_4
#define SD_CS_GPIO_Port GPIOA
#define SD_SCK_Pin GPIO_PIN_5
#define SD_SCK_GPIO_Port GPIOA
#define SD_MISO_Pin GPIO_PIN_6
#define SD_MISO_GPIO_Port GPIOA
#define SD_MOSI_Pin GPIO_PIN_7
#define SD_MOSI_GPIO_Port GPIOA
#define DEBUG_LED_Pin GPIO_PIN_0
#define DEBUG_LED_GPIO_Port GPIOB
#define EPD_BUSY_Pin GPIO_PIN_1
#define EPD_BUSY_GPIO_Port GPIOB
#define SD_CD_Pin GPIO_PIN_8
#define SD_CD_GPIO_Port GPIOA
#define EPD_CS_Pin GPIO_PIN_15
#define EPD_CS_GPIO_Port GPIOA
#define EPD_SCK_Pin GPIO_PIN_3
#define EPD_SCK_GPIO_Port GPIOB
#define EPD_DC_Pin GPIO_PIN_4
#define EPD_DC_GPIO_Port GPIOB
#define EPD_MOSI_Pin GPIO_PIN_5
#define EPD_MOSI_GPIO_Port GPIOB
#define EPD_RES_Pin GPIO_PIN_6
#define EPD_RES_GPIO_Port GPIOB
#define SD_ON_Pin GPIO_PIN_7
#define SD_ON_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
