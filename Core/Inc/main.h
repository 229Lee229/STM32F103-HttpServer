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
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */


// 宏定义（方便修改�?
#define RS485_DE_GPIO_Port    GPIOC
#define RS485_DE_Pin          GPIO_PIN_5

// 发�?�前调用：进入发送模�?
#define RS485_TX_ENABLE()   HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_SET)

// 发�?�完成后调用：切回接收模�?
#define RS485_RX_ENABLE()   HAL_GPIO_WritePin(RS485_DE_GPIO_Port, RS485_DE_Pin, GPIO_PIN_RESET)
void my_rtu_abort_receive(void);
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

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define UART2_T_RTU_temp_Pin GPIO_PIN_2
#define UART2_T_RTU_temp_GPIO_Port GPIOA
#define UART2_R_RTU_temp_Pin GPIO_PIN_3
#define UART2_R_RTU_temp_GPIO_Port GPIOA
#define RS485_EN_Pin GPIO_PIN_5
#define RS485_EN_GPIO_Port GPIOC
#define LED_R_Pin GPIO_PIN_0
#define LED_R_GPIO_Port GPIOB
#define UART3_T_debug_Pin GPIO_PIN_10
#define UART3_T_debug_GPIO_Port GPIOB
#define UART3_R_debug_Pin GPIO_PIN_11
#define UART3_R_debug_GPIO_Port GPIOB
#define W5500_RST_Pin GPIO_PIN_15
#define W5500_RST_GPIO_Port GPIOG
#define W5500_CSS_Pin GPIO_PIN_6
#define W5500_CSS_GPIO_Port GPIOB
#define W5500_INT_Pin GPIO_PIN_7
#define W5500_INT_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
