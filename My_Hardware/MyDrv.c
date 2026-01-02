#include "MyDrv.h"
#include "main.h"
#include "w5500.h"
#include "wizchip_conf.h"
extern SPI_HandleTypeDef * const p_hspi_w5500;
// extern SPI_HandleTypeDef hspi1;
// SPI_HandleTypeDef * const pW5500Spi = &hspi1;
/**

* @brief W5500 SPI single-byte read callback function

* @note Reads a byte from the W5500 by sending a dummy byte (0xFF).

* This function is automatically called by the ioLibrary_Driver library on all register/buffer read operations.

* Must be used in pair with my_wizchip_spi_writebyte and executed within a critical section (CS pulled low).

* @param None

* @retval uint8_t Byte data read from the W5500

*/
uint8_t my_wizchip_spi_readbyte(void)
{
    uint8_t rx_byte = 0xFF;  // dummy byte
    HAL_SPI_TransmitReceive(p_hspi_w5500, &rx_byte, &rx_byte, 1, HAL_MAX_DELAY);
    return rx_byte;
}

/**

* @brief W5500 SPI single-byte write callback function

* @note Sends a byte of data to the W5500.

* This function is automatically called by the ioLibrary_Driver library on all register/buffer write operations.

* Must be used in pair with my_wizchip_spi_readbyte and executed within a critical section (CS pulled low).

* @param wb The byte of data to write to the W5500

* @retval None

*/
void my_wizchip_spi_writebyte(uint8_t wb)
{
    HAL_SPI_Transmit(p_hspi_w5500, &wb, 1, HAL_MAX_DELAY);
}


void SPI_CrisEnter(void)
{
    __set_PRIMASK(1);
}

void SPI_CrisExit(void)
{
    __set_PRIMASK(0);
}
 
void SPI_CS_Select(void)
{
    HAL_GPIO_WritePin(W5500_NSS_GPIO_Port, W5500_NSS_Pin, GPIO_PIN_RESET);
}

void SPI_CS_Deselect(void)
{
    HAL_GPIO_WritePin(W5500_NSS_GPIO_Port, W5500_NSS_Pin, GPIO_PIN_SET);
}

void register_wizchip()
{
	// First of all, Should register SPI callback functions implemented by user for accessing WIZCHIP 
	
	/* Critical section callback */
	/* Register critical section callback (protect SPI communication from interruption) */
	reg_wizchip_cris_cbfunc(SPI_CrisEnter, SPI_CrisExit);   
	
	/* Chip selection call back */
	reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect);
	
	/* SPI Read & Write callback function */
	reg_wizchip_spi_cbfunc(my_wizchip_spi_readbyte, my_wizchip_spi_writebyte);  
	
	// Optional: If burst read/write is implemented, you can also register (for better performance).
	// reg_wizchip_spi_burst_cbfunc(my_wizchip_spi_readburst, my_wizchip_spi_writeburst);
}
