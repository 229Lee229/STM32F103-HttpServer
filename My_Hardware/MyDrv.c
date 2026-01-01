#include "main.h"
#include "W5500.H"
/**
  * @brief  ?????
  * @retval None
  */
void SPI_CrisEnter(void)
{
    __set_PRIMASK(1);
}
/**
  * @brief  ?????
  * @retval None
  */
void SPI_CrisExit(void)
{
    __set_PRIMASK(0);
}
 
/**
  * @brief  ?????????
  * @retval None
  */
void SPI_CS_Select(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
}
/**
  * @brief  ?????????
  * @retval None
  */
void SPI_CS_Deselect(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}
void register_wizchip()
{
	// First of all, Should register SPI callback functions implemented by user for accessing WIZCHIP 
	/* Critical section callback */
	reg_wizchip_cris_cbfunc(SPI_CrisEnter, SPI_CrisExit);   
	/* Chip selection call back */
	reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect);
	
	/* SPI Read & Write callback function */
	// reg_wizchip_spi_cbfunc(SPI1_ReadByte, SPI1_WriteByte);  
}
