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
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "socket.h"
#include "modbus.h"
#include <w5500.h>
#include <wizchip_conf.h>
#include "Conf_SPI_W5500.h"
#include "errno.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
static uint8_t tcp_rx_buf[MODBUS_TCP_MAX_ADU];   // TCP 接收缓冲
static uint8_t rtu_tx_buf[MODBUS_RTU_MAX_ADU];   // RTU 发送缓冲（请求）
static uint8_t rtu_rx_buf[MODBUS_RTU_MAX_ADU];   // RTU 接收缓冲（响应）
static uint8_t rtu_req_buf[MODBUS_RTU_MAX_ADU];  // RTU 请求缓冲
static uint16_t sensor_vals[10];
static modbus_t *mb_tcp_ctx = NULL;
static modbus_mapping_t *mb_mapping = NULL;			// register mapping
uint16_t g_sensor_temp = 0;
uint16_t g_sensor_humi = 0;


SPI_HandleTypeDef * const p_hspi_w5500 = &hspi1;
		wiz_NetInfo gWIZNETINFO;		// setINFO
		wiz_NetInfo netinfo;			// readback

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

void ModbusTCP_Server(void);
static void LibmodbusClientTest(void);
void Load_Net_Parameters(void);
uint8_t W5500_WaitReady(uint16_t timeout_ms);
void ModbusTCPSlaveTask(void);
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

void my_rtu_abort_receive(void)
{
    
    HAL_UART_AbortReceive_IT(&huart2);  // 关键：中止中断接�?
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
		HAL_GPIO_TogglePin(LED_R_GPIO_Port,LED_R_Pin);
		printf("keep going from ZET6.\r\n");
		// HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_5);
		
		// LibmodbusClientTest();
		// ifmodbus_read_input_registers(ctx, 0, 2, vals);
		
    }
}
static void LibmodbusClientTest(void)	
{
	modbus_t *ctx;
	int rc;
	int rc2;
	uint16_t vals[2];
	
	ctx = modbus_new_st_rtu("uart2", 9600, 'N', 8, 1);
	modbus_set_slave(ctx, 1);
	//modbus_rtu_set_serial_mode(ctx,MODBUS_RTU_RS485);
	//modbus_rtu_set_rts(ctx, MODBUS_RTU_RTS_UP);        // 高电平使能发�?
	//modbus_rtu_set_rts_delay(ctx, 500);                // 500 μs 延时
	modbus_set_debug(ctx, TRUE);
	modbus_set_response_timeout(ctx, 0, 500000);   // 
		modbus_set_byte_timeout(ctx, 0, 2000);   // 2 ms，常用安全�??

	rc = modbus_connect(ctx);
	if (rc == -1) {
		//fprintf(stderr, "Unable to connect %s\n", modbus_strerror(errno));
		printf("Unable to connect Slave");
		modbus_free(ctx);
	}


	// test get timeout
	uint32_t sec,usec;
	modbus_get_response_timeout(ctx,&sec,&usec);
	printf("modbus_get_response_timeout:sec:%u, usec:%u\r\n",sec,usec);

	for (;;) {
			  
		// rc = modbus_read_input_registers(ctx, 0, 2, vals);
		rc = modbus_read_registers(ctx, 0, 2, vals);
		// rc2 = modbus_read_input_registers(ctx, 1, 1, vals2);
		
		printf("rc = %d\r\n",rc);
		// printf("waitting");
		if (rc == 2)
		{
		    // printf("TEM/HUM Sensor : temp %d.%d, humi %d.%d          \r\n", vals[0]/10, vals[0]%10, vals[1]/10, vals[1]%10);
			g_sensor_temp = vals[0];
			g_sensor_humi = vals[1];
			printf("TEMP:%d.%d\r\n", g_sensor_temp/10,g_sensor_temp%10);
			printf("HUMI: %d.%d\r\n",g_sensor_humi/10,g_sensor_humi%10);
		}

         HAL_Delay(900);
	}

	/* For RTU */
	modbus_close(ctx);
	modbus_free(ctx);
}

// 1. �? main.c 或合适地方，重写弱定义的回调（最干净�?
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        // 只有真正发�?�完成（�?后一个字�?+停止位发出）才会进这�?
        // RS485_RX_ENABLE();          // 切回接收
		//printf("ok\r\n");
        // 可�?�：清一些发送完成标�? send_complete = 1;
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
//int sub(int a,int b){
//	return a - b;
//}

//int add(int a,int b){
//	return a+b;
//}
//typedef int(*pfun)(int,int);

//int calc(pfun fp,int a,int b){
//	return fp(a,b);
//}



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
//	int x = 5, j = 2;
//	int z;
//	z = calc(add,5,2);
//	return 0;
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
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_TIM2_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
	HAL_TIM_Base_Start_IT(&htim2);
	

	HAL_UART_Receive_IT(&huart2, &rx_byte, 1);  // rx_byte ??? uint8_t
	HAL_UART_AbortReceive_IT(&huart2);

	
	uint8_t memsize[2][8] = { {2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};

	register_wizchip();
	Load_Net_Parameters();
		
	if (W5500_WaitReady(455) == 0)  // 轮询 100ms 超时
    {
        printf("W5500 init timeout! Check hardware.\r\n");
        NVIC_SystemReset();  // 立即软复位，整个 MCU 复位
		// while(1);  // 失败死循环
    }
	printf("W5500 ready! VERSIONR = 0x%02X\r\n", getVERSIONR());
	// while(!(HAL_GPIO_ReadPin(W5500_RST_GPIO_Port,W5500_RST_Pin))){}	
	// HAL_Delay(200);
		wizchip_setnetinfo(&gWIZNETINFO);
	
		wizchip_getnetinfo(&netinfo);
				// Determine if initialization was successful

		if(ctlwizchip(CW_INIT_WIZCHIP, (void*)memsize) == -1)
	{
		printf("WIZCHIP Initialized fail.\r\n");
		while(1);
	}
	
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
	
	

		


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  printf("Hello, from ZET6\r\n");
 	  // LibmodbusClientTest();

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	   // LibmodbusClientTest();
	  ModbusTCP_Server();
	HAL_Delay(500);
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


/**
  * @brief  Modbus TCP 服务器主循环
  *         监听 502 端口，接收请求 → 转发 RTU → 构造响应返回
  */
void ModbusTCP_Server(void)
{
    int32_t sock = socket(0, Sn_MR_TCP, MODBUS_TCP_PORT, 0);
    if (sock < 0)
    {
        printf("TCP socket create failed: %d\r\n", sock);
        return;
    }

    if (listen(sock) != SOCK_OK)
    {
        printf("TCP listen failed\r\n");
        close(sock);
        return;
    }

    printf("Modbus TCP listening on port %d...\r\n", MODBUS_TCP_PORT);

    while (1)
    {
        uint8_t sr = getSn_SR(sock);
        if (sr == SOCK_ESTABLISHED)
        {
            uint16_t rx_len = getSn_RX_RSR(sock);
            if (rx_len > 0)
            {
                // 接收 Modbus TCP 请求帧
                int32_t recv_len = recv(sock, tcp_rx_buf, MODBUS_TCP_MAX_ADU);
                if (recv_len <= 7)  // 至少要有 MBAP 头 7 字节
                {
                    continue;
                }

                // 解析 MBAP 头
                uint16_t transaction_id = (tcp_rx_buf[0] << 8) | tcp_rx_buf[1];
                uint16_t protocol_id    = (tcp_rx_buf[2] << 8) | tcp_rx_buf[3];
                uint16_t length         = (tcp_rx_buf[4] << 8) | tcp_rx_buf[5];
                uint8_t  unit_id        = tcp_rx_buf[6];

                // 基本校验
                if (protocol_id != 0 || length < 2 || length > MODBUS_TCP_MAX_ADU_LENGTH - 7)
                {
                    close(sock);
                    continue;
                }

                // 提取 PDU（从第 7 字节开始）
                uint8_t *pdu = &tcp_rx_buf[7];
                uint16_t pdu_len = length - 1;  // 减去 unit_id

                printf("Received Modbus TCP: TID=%04X, Unit=%d, PDU len=%d\r\n",
                       transaction_id, unit_id, pdu_len);

                // 创建临时的 libmodbus RTU 上下文（只用于解析/构建 PDU）
                modbus_t *rtu_ctx = modbus_new_st_rtu("uart2", 9600, 'N', 8, 1);
                if (rtu_ctx == NULL)
                {
                    printf("RTU ctx create failed\r\n");
                    continue;
                }

                modbus_set_slave(rtu_ctx, unit_id);
				modbus_set_debug(rtu_ctx, TRUE);
                modbus_set_response_timeout(rtu_ctx, 0, 600000);   // 500ms，与你 RTU 示例一致
                modbus_set_byte_timeout(rtu_ctx, 0, 20000);
				
				
				// 提取请求中的起始地址和数量（假设功能码 0x03 读保持寄存器）
                if (pdu[0] == 0x03 && pdu_len >= 5)  // 功能码 + 地址(2B) + 数量(2B)
                {
                    uint16_t start_addr = (pdu[1] << 8) | pdu[2];
                    uint16_t nb         = (pdu[3] << 8) | pdu[4];

                    // 调用你熟悉的 modbus_read_registers（与 RTU 示例完全一样）
                    int rc = modbus_read_registers(rtu_ctx, start_addr, nb, sensor_vals);

                    printf("TCPrc = %d\r\n", rc);

                    if (rc == nb)  // 成功读取 nb 个寄存器
                    {
                        // 构造响应 PDU
                        uint8_t resp_pdu[MODBUS_TCP_MAX_ADU];
                        uint16_t resp_pdu_len = 0;

                        resp_pdu[resp_pdu_len++] = 0x03;             // 功能码
                        resp_pdu[resp_pdu_len++] = nb * 2;           // 字节计数

                        // 填充数据（大端序）
                        for (int i = 0; i < nb; i++)
                        {
                            resp_pdu[resp_pdu_len++] = sensor_vals[i] >> 8;
                            resp_pdu[resp_pdu_len++] = sensor_vals[i] & 0xFF;
                        }

                        // 构造完整 TCP 响应帧
                        uint8_t resp_buf[MODBUS_TCP_MAX_ADU];
                        uint16_t resp_len = 0;

                        // MBAP 头：复制请求的前 4 字节（TID + Protocol ID）
                        memcpy(resp_buf, tcp_rx_buf, 4);
                        resp_len += 4;

                        // Length = PDU长度 + 1（Unit ID）
                        uint16_t mbap_len = resp_pdu_len + 1;
                        resp_buf[resp_len++] = mbap_len >> 8;
                        resp_buf[resp_len++] = mbap_len & 0xFF;

                        // Unit ID
                        resp_buf[resp_len++] = unit_id;

                        // PDU
                        memcpy(&resp_buf[resp_len], resp_pdu, resp_pdu_len);
                        resp_len += resp_pdu_len;

                        // 发送响应（使用你提供的 send 函数）
                        send(sock, resp_buf, resp_len);
                        printf("Sent Modbus TCP response, len=%d\r\n", resp_len);
							printf("Resp_buf: \r\n");
							printf("-->>>>>>> ");
						for(int i = 0;i < resp_len;i++){
							printf("%02x ",resp_buf[i]);
						}
						printf(" <<<<<<<<--\r\n");
						// printf("Resp_buf: ");
                    }
                    else
                    {
                        printf("RTU read failed: %s\r\n", modbus_strerror(errno));
                    }
                }
                else
                {
                    printf("Unsupported function code or request format\r\n");
                }

                modbus_free(rtu_ctx);
            
            


		}
	}
        // 处理连接状态变化
        uint8_t sr2 = getSn_SR(sock);
        if (sr2 == SOCK_CLOSE_WAIT || sr2 == SOCK_CLOSED || sr2 == SOCK_LAST_ACK)
        {
            close(sock);
            // 重新创建 socket 继续监听
            sock = socket(0, Sn_MR_TCP, MODBUS_TCP_PORT, 0);
            listen(sock);
        }
		HAL_Delay(1);

	}
}

// Modbus TCP Slave 任务（放在主循环或独立任务中）
//void ModbusTCPSlaveTask(void)
//{
//    // 创建 TCP Slave，监听所有接口，端口 502
//    mb_tcp_ctx = modbus_new_tcp("0.0.0.0", 502);
//    if (mb_tcp_ctx == NULL)
//    {
//        printf("Modbus TCP create failed: %s\r\n", modbus_strerror(errno));
//        return;
//    }

//    // 创建寄存器映射（这里映射 100 个保持寄存器，供上位机读写）
//    mb_mapping = modbus_mapping_new(0, 0, 100, 0);  // 0 coil, 0 discrete, 100 holding, 0 input
//    if (mb_mapping == NULL)
//    {
//        printf("Modbus mapping failed: %s\r\n", modbus_strerror(errno));
//        modbus_free(mb_tcp_ctx);
//        return;
//    }

//    // 把传感器数据映射到保持寄存器（例如地址 0=温度, 1=湿度）
//    while (1)
//    {
//        // 实时更新缓存到 Modbus 寄存器
//        mb_mapping->tab_registers[0] = g_sensor_temp;   // 温度 ×10
//        mb_mapping->tab_registers[1] = g_sensor_humi;   // 湿度 ×10

//        // 监听并处理一个 TCP 连接
//        int sock = modbus_tcp_listen(mb_tcp_ctx, 1);  // 监听，允许 1 个连接
//        if (sock == -1)
//        {
//            printf("Modbus TCP listen failed: %s\r\n", modbus_strerror(errno));
//            continue;
//        }

//        modbus_tcp_accept(mb_tcp_ctx, &sock);  // 接受连接

//        // 循环处理请求（直到客户端断开）
//        while (1)
//        {
//            uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
//            int rc = modbus_receive(mb_tcp_ctx, query);
//            if (rc > 0)
//            {
//                // 处理请求并回复
//                modbus_reply(mb_tcp_ctx, query, rc, mb_mapping);
//            }
//            else if (rc == -1)
//            {
//                // 客户端断开或错误
//                break;
//            }
//        }

//        modbus_close(mb_tcp_ctx);  // 关闭当前连接，继续监听下一个
//    }

//    // 清理（正常不会走到这里）
//    modbus_mapping_free(mb_mapping);
//    modbus_free(mb_tcp_ctx);
//}
// 判断 W5500 就绪的轮询函数
uint8_t W5500_WaitReady(uint16_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    uint8_t version = 0;

    while (HAL_GetTick() - start < timeout_ms)
    {
        version = getVERSIONR();  // 读取 VERSIONR 寄存器
        if (version == 0x04)
        {
            return 1;  // 就绪成功
        }
        HAL_Delay(1);  // 短延时重试
    }
    return 0;  // 超时失败
}

void Load_Net_Parameters(void)
{
	gWIZNETINFO.gw[0] = 192; //Gateway
	gWIZNETINFO.gw[1] = 168;
	gWIZNETINFO.gw[2] = 99;
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
	gWIZNETINFO.mac[5]=0x12;

	gWIZNETINFO.ip[0]=192; //IP
	gWIZNETINFO.ip[1]=168;
	gWIZNETINFO.ip[2]=99;
	gWIZNETINFO.ip[3]=199;
	
	gWIZNETINFO.dns[0] = 192;
	gWIZNETINFO.dns[1] = 168;
	gWIZNETINFO.dns[2] = 99;
	gWIZNETINFO.dns[3] = 1;	
	gWIZNETINFO.dhcp = NETINFO_STATIC;
}


// 在主循环或任务中
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
