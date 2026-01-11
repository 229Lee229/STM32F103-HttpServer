/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "modbus.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
uint8_t rx_byte;
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 100);
    return ch;
}

// ???? Keil ???? fputc(???????)
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 100);
    return ch;
}
HAL_StatusTypeDef UART_SendString(const char* str)
{
    uint16_t len = 0;
    while (str[len] != '\0') len++;  // ???????

    return HAL_UART_Transmit(&huart3, (uint8_t*)str, len, HAL_MAX_DELAY);  // ????
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim2)  // ????????(????????????)
    {
        // ?????????
        // ??:?? LED?????????
		//   HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);
		// HAL_GPIO_TogglePin(LED_R_GPIO_Port,LED_R_Pin);
		printf("keep going from ZET6.\r\n");
		// HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_5);
    }
}
static void LibmodbusClientTest(void)	
{
	modbus_t *ctx;
	int rc;
	int rc2;
	uint16_t vals[2],vals2[2];
	
	ctx = modbus_new_st_rtu("uart2", 9600, 'N', 8, 1);
	modbus_set_slave(ctx, 1);
	modbus_set_debug(ctx, TRUE);
	rc = modbus_connect(ctx);
	if (rc == -1) {
		//fprintf(stderr, "Unable to connect %s\n", modbus_strerror(errno));
		printf("Unable to connect Slave");
		modbus_free(ctx);
	}


	for (;;) {
			  
		rc = modbus_read_input_registers(ctx, 0, 1, vals);
		rc2 = modbus_read_input_registers(ctx, 1, 1, vals2);
		
		// printf("rc = %d\r\n",rc);
		// printf("waitting");
		if (rc == 1)
		{
		    // printf("TEM/HUM Sensor : temp %d.%d, humi %d.%d          \r\n", vals[0]/10, vals[0]%10, vals[1]/10, vals[1]%10);
			printf("TEMP:%d.%d\r\n", vals[0]/10,vals[0]%10);
			printf("HUM: %d.%d\r\n", vals2[0]/10,vals2[0]%10);
		}

        HAL_Delay(500);
	}

	/* For RTU */
	modbus_close(ctx);
	modbus_free(ctx);
}

// 1. 在 main.c 或合适地方，重写弱定义的回调（最干净）
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        // 只有真正发送完成（最后一个字节+停止位发出）才会进这里
        // RS485_RX_ENABLE();          // 切回接收
		printf("ok\r\n");
        // 可选：清一些发送完成标志 send_complete = 1;
    }
}

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
	HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_SET);
	MX_USART2_UART_Init();
	MX_USART3_UART_Init();
	MX_TIM2_Init();
	/* USER CODE BEGIN 2 */
		HAL_TIM_Base_Start_IT(&htim2);

	HAL_UART_Receive_IT(&huart2, &rx_byte, 1);  // rx_byte ??? uint8_t
	// __HAL_UART_ENABLE_IT(&huart2, UART_IT_TC);  // huart2 是你的 UART 句柄
	// uint32_t cr1 = USART2->CR1;
//	if (cr1 & USART_CR1_TCIE) {
//			HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin, GPIO_PIN_RESET);

//	// 成功开启了 TCIE
//	// 可以打日志或亮灯
//	} else {
//	// 失败！说明 ENABLE_IT 没生效
//	// 可能时钟没开、串口没初始化、参数错
//	}
	HAL_UART_Receive_IT(&huart3, &rx_byte, 1);  // rx_byte ??? uint8_t

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  printf("Hello, from ZET6\r\n");
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  LibmodbusClientTest();
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
