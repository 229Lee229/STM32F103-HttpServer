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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "httpServer.h"
#include "httpParser.h"
#include "httpUtil.h"
#include "w5500.h"
#include "socket.h"

#include "wizchip_conf.h"
#include "MyDrv.h"
#include "stdio.h"
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
SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
SPI_HandleTypeDef * const p_hspi_w5500 = &hspi1;
// SPI_HandlePtr hspi_w5500 = &hspi1;   // 用于 W5500

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#define RING_BUF_SIZE 2048  // ?? SRAM ??,F103 ?? 1~4KB
uint8_t my_wizchip_spi_readbyte(void);
void my_wizchip_spi_writebyte(uint8_t wb);

typedef struct
{
    uint8_t buf[RING_BUF_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} RingBuffer;

RingBuffer uart_ring = {0};

void RingBuffer_Write(uint8_t data)
{
    uint16_t next = (uart_ring.head + 1) % RING_BUF_SIZE;
    if (next != uart_ring.tail)  // ??
    {
        uart_ring.buf[uart_ring.head] = data;
        uart_ring.head = next;
    }
    // ???????????(????)
}

uint16_t RingBuffer_Available(void)
{
    return (uart_ring.head - uart_ring.tail + RING_BUF_SIZE) % RING_BUF_SIZE;
}

uint8_t RingBuffer_Read(void)
{
    uint8_t data = uart_ring.buf[uart_ring.tail];
    uart_ring.tail = (uart_ring.tail + 1) % RING_BUF_SIZE;
    return data;
}





uint8_t rx_byte;
// ??? printf ? UART3
int __io_putchar(int ch)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}

// ???? Keil ???? fputc(???????)
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}


/**
  * @brief  UART ?????(????)
  * @param  str: ???????(? \0 ??)
  * @retval HAL_StatusTypeDef: HAL_OK ??
  */
HAL_StatusTypeDef UART_SendString(const char* str)
{
    uint16_t len = 0;
    while (str[len] != '\0') len++;  // ???????

    return HAL_UART_Transmit(&huart3, (uint8_t*)str, len, HAL_MAX_DELAY);  // ????
}

/**
  * @brief  UART ????????(????)
  * @param  data: ????
  * @param  len: ????
  * @retval HAL_StatusTypeDef
  */
HAL_StatusTypeDef UART_SendData(const uint8_t* data, uint16_t len)
{
    return HAL_UART_Transmit(&huart3, data, len, HAL_MAX_DELAY);
}

/**
  * @brief  UART ??????(??)
  * @param  byte: ??????
  */
HAL_StatusTypeDef UART_SendByte(uint8_t byte)
{
    return HAL_UART_Transmit(&huart3, &byte, 1, HAL_MAX_DELAY);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart3)
    {
		RingBuffer_Write(rx_byte);
        // ?????? rx_byte
        // ??????
		
        HAL_UART_Receive_IT(&huart3, &rx_byte, 1);
    }
}


// ??????????(???)
void ProcessLongString(void)
{
    static uint8_t long_str[4096];  // ?????
    static uint16_t pos = 0;

    while (RingBuffer_Available() > 0)
    {
        uint8_t ch = RingBuffer_Read();
        if (ch == '\n' || ch == '\0' || pos >= sizeof(long_str)-1)  // ????
        {
            long_str[pos] = '\0';
            // ???????
            printf("Received long string: %s\r\n", long_str);
            // ??
            pos = 0;
            break;
        }
        else
        {
            long_str[pos++] = ch;
        }
    }
}


/**
  * @brief  TIM2 ????????
  *         ?????? Period ?????
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim2)  // ????????(????????????)
    {
        // ?????????
        // ??:?? LED?????????
		printf("Ready.\r\n");
    }
}

/**
  * @brief  SPI ????(????)
  * @param  data: ????????
  * @param  len: ????
  * @retval HAL_OK: ??
  */
HAL_StatusTypeDef SPI_Transmit(uint8_t* data, uint16_t len)
{
    return HAL_SPI_Transmit(&hspi1, data, len, HAL_MAX_DELAY);
}

/**
  * @brief  SPI ????(????)
  * @param  data: ???????
  * @param  len: ??????
  * @retval HAL_OK: ??
  */
HAL_StatusTypeDef SPI_Receive(uint8_t* data, uint16_t len)
{
    return HAL_SPI_Receive(&hspi1, data, len, HAL_MAX_DELAY);
}

/**
  * @brief  SPI ???????(???!W5500 ????????)
  * @param  tx_data: ??????(????,?? 0xFF)
  * @param  rx_data: ???????
  * @param  len: ????
  * @retval HAL_OK: ??
  */
HAL_StatusTypeDef SPI_TransmitReceive(uint8_t* tx_data, uint8_t* rx_data, uint16_t len)
{
    return HAL_SPI_TransmitReceive(&hspi1, tx_data, rx_data, len, HAL_MAX_DELAY);
}





wiz_NetInfo gWIZNETINFO;		// setINFO
wiz_NetInfo netinfo;			// readback
void Load_Net_Parameters(void)
{
	gWIZNETINFO.gw[0] = 192; //Gateway
	gWIZNETINFO.gw[1] = 168;
	gWIZNETINFO.gw[2] = 101;
	gWIZNETINFO.gw[3] = 1;

	gWIZNETINFO.sn[0]=255; //Mask
	gWIZNETINFO.sn[1]=255;
	gWIZNETINFO.sn[2]=255;
	gWIZNETINFO.sn[3]=0;

	gWIZNETINFO.mac[0]=0x0c; //MAC
	gWIZNETINFO.mac[1]=0x29;
	gWIZNETINFO.mac[2]=0xab;
	gWIZNETINFO.mac[3]=0x7c;
	gWIZNETINFO.mac[4]=0x00;
	gWIZNETINFO.mac[5]=0x09;

	gWIZNETINFO.ip[0]=192; //IP
	gWIZNETINFO.ip[1]=168;
	gWIZNETINFO.ip[2]=101;
	gWIZNETINFO.ip[3]=99;
	
	gWIZNETINFO.dns[0] = 8;
	gWIZNETINFO.dns[1] = 8;
	gWIZNETINFO.dns[2] = 8;
	gWIZNETINFO.dns[3] = 8;	
	gWIZNETINFO.dhcp = NETINFO_STATIC;
}




static uint8_t http_tx_buf[HTTP_TX_BUF_SIZE];
static uint8_t http_rx_buf[HTTP_RX_BUF_SIZE];
static uint8_t http_socket_num[MAX_HTTPSOCK] = {HTTP_SOCKET};

uint8_t * get_http_rx_buffer(void){
	return http_rx_buf;
}
uint16_t  get_http_rx_buffer_size(void){
	return HTTP_RX_BUF_SIZE;
}

const uint8_t http_index_html[] = 
"<html><head><title>W5500 HTTP Server</title></head>"
"<body><h1>Hello from STM32 + W5500   :)	!</h1>"
"<p>Current time: %s</p></body></html>";
const uint16_t http_index_html_len = sizeof(http_index_html) - 1;  // ?????\0


// Define a simple index.html content (uint8_t array)
const uint8_t index_html[] = {
    '<', '!', 'D', 'O', 'C', 'T', 'Y', 'P', 'E', ' ', 'h', 't', 'm', 'l', '>', '\r', '\n',
    '<', 'h', 't', 'm', 'l', '>', '\r', '\n',
    '<', 'h', 'e', 'a', 'd', '>', '<', 't', 'i', 't', 'l', 'e', '>', 'W', '5', '5', '0', '0', ' ', 'S', 'e', 'r', 'v', 'e', 'r', '<', '/', 't', 'i', 't', 'l', 'e', '>', '<', '/', 'h', 'e', 'a', 'd', '>', '\r', '\n',
    '<', 'b', 'o', 'd', 'y', '>', '\r', '\n',
    '<', 'h', '1', '>', 'H', 'e', 'l', 'l', 'o', ' ', 'f', 'r', 'o', 'm', ' ', 'A', 'P', 'M', '3', '2', ' ', '+', ' ', 'W', '5', '5', '0', '0', '!', '<', '/', 'h', '1', '>', '\r', '\n',
    '<', 'p', '>', 'T', 'h', 'i', 's', ' ', 'i', 's', ' ', 'a', ' ', 'c', 'u', 's', 't', 'o', 'm', ' ', 'p', 'a', 'g', 'e', '.', '<', '/', 'p', '>', '\r', '\n',
    '<', '/', 'b', 'o', 'd', 'y', '>', '\r', '\n',
    '<', '/', 'h', 't', 'm', 'l', '>', '\0'
};

// 1. ??????(? index.html ??)
const uint8_t index_html_content[] = 
"<!DOCTYPE html>\r\n"
"<html><head><title>My Device</title></head>\r\n"
"<body>\r\n"
"<h1>Hello! W5500 HTTP Server is running.</h1>\r\n"
"<p>Current time: 2026-01-02</p>\r\n"
"<a href=\"/data.json\">View real-time data (JSON)</a>\r\n"
"</body></html>\r\n";

// 2. ?? JSON ????
const uint8_t data_json_content[] = 
"{\r\n"
"  \"voltage\": 220.45,\r\n"
"  \"current\": 5.12,\r\n"
"  \"power\": 1125.4,\r\n"
"  \"timestamp\": \"2026-01-02 14:30:00\"\r\n"
"}\r\n";

const uint8_t data_json_content2[] = 
"{\r\n"
"  \"Example\": a,\r\n"
"  \"Example\": b,\r\n"
"  \"Name\": King,\r\n"
"  \"timestamp\": \"2026-01-02 23:30:00\"\r\n"
"}\r\n";
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
  MX_SPI1_Init();
  MX_USART3_UART_Init();
  MX_TIM2_Init();
  
     LOG_INFO("program start.");
    

  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart3, &rx_byte, 1);  // rx_byte ??? uint8_t
  HAL_TIM_Base_Start_IT(&htim2);
  uint8_t memsize[2][8] = { {2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};

	register_wizchip();
	Load_Net_Parameters();
	
	  
	  
//	  uint8_t read_mac[6];
//	getSHAR(read_mac);  // ioLibrary_Driver ?????

//printf("Current MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
//       read_mac[0], read_mac[1], read_mac[2],
//       read_mac[3], read_mac[4], read_mac[5]);
//	
//	
//	 setSHAR(gWIZNETINFO.mac);
//		getSHAR(read_mac);  // ioLibrary_Driver ?????

//printf("AfterCurrent MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
//       read_mac[0], read_mac[1], read_mac[2],
//       read_mac[3], read_mac[4], read_mac[5]);

  /* WIZCHIP SOCKET Buffer initialize */
  
  // uint8_t memsize[2][8] = {{2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};  // ?????

if(ctlwizchip(CW_INIT_WIZCHIP, (void*)memsize) == -1)
{
    printf("WIZCHIP Initialized fail.\r\n");
    while(1);
}
printf("WIZCHIP Initialized success.\r\n");
//  if(ctlwizchip(0, (void*)memsize) == -1){
//    printf("WIZCHIP Initialized fail.\r\n");
//    while(1);
//  }

	uint8_t read_ip[4];

//	getSIPR(read_ip);
//printf("Current IP: %02d:%02d:%02d:%02d\r\n",
//       read_ip[0], read_ip[1], read_ip[2],
//       read_ip[3]);



//	setSIPR(gWIZNETINFO.ip);
//	getSIPR(read_ip);
//	printf("AfterCurrent IP: %02d:%02d:%02d:%02d\r\n",
//       read_ip[0], read_ip[1], read_ip[2],
//       read_ip[3]);


	wizchip_setnetinfo(&gWIZNETINFO);
	
	wizchip_getnetinfo(&netinfo);
	
	
	printf("Current network info:\r\n");
printf("MAC : %02X:%02X:%02X:%02X:%02X:%02X\r\n",
       netinfo.mac[0], netinfo.mac[1], netinfo.mac[2],
       netinfo.mac[3], netinfo.mac[4], netinfo.mac[5]);

printf("IP  : %d.%d.%d.%d\r\n",
       netinfo.ip[0], netinfo.ip[1], netinfo.ip[2], netinfo.ip[3]);

printf("SN  : %d.%d.%d.%d\r\n",
       netinfo.sn[0], netinfo.sn[1], netinfo.sn[2], netinfo.sn[3]);

printf("GW  : %d.%d.%d.%d\r\n",
       netinfo.gw[0], netinfo.gw[1], netinfo.gw[2], netinfo.gw[3]);

printf("DNS : %d.%d.%d.%d\r\n",
       netinfo.dns[0], netinfo.dns[1], netinfo.dns[2], netinfo.dns[3]);
	
	
	
	
	// ctlnetwork();
	
	
	httpServer_init(http_tx_buf, http_rx_buf, MAX_HTTPSOCK, http_socket_num);
	// reg_httpServer_webpage((uint8_t*)http_index_html);
	// set_httpServer_webcontent((uint8_t*)"index.html", (uint8_t*)http_index_html);
	// reg_httpServer_webContent((uint8_t*)"index.html", (uint8_t*)http_index_html);
	reg_httpServer_webContent((uint8_t*)"index.html", (uint8_t*)index_html_content);
	reg_httpServer_webContent((uint8_t*)"data.json",  (uint8_t*)data_json_content);
	// reg_httpServer_webContent((uint8_t*)"status",  (uint8_t*)data_json_content2);

	//		reg_httpServer_webContent((uint8_t*)"example.cgi",  (uint8_t*)data_json_content2);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	UART_SendString("Hello from STM32!\r\n");
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  httpServer_run(http_socket_num[0]);  
	  	// UART_SendString("Hello from STM32!\r\n");
	// printf("Hello 2026!");
	// HAL_Delay(500);
	  ProcessLongString();
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

  /** Enables the Clock Security System
  */
  HAL_RCC_EnableCSS();
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
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7199;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9999;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(W5500_NSS_GPIO_Port, W5500_NSS_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : W5500_NSS_Pin */
  GPIO_InitStruct.Pin = W5500_NSS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(W5500_NSS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : W5500_RST_Pin */
  GPIO_InitStruct.Pin = W5500_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(W5500_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : W5500_INT_Pin */
  GPIO_InitStruct.Pin = W5500_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(W5500_INT_GPIO_Port, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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
