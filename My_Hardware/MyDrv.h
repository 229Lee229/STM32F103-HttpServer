#ifndef __MYDRV_H
#define __MYDRV_H

#include "main.h"

// Debugging phase: To quickly verify whether communication is working properly, use blocking mode.
#define SPI_TIMEOUT_BLOCKING	

// Safe phase
#define SPI_TIMEOUT_SAFE

/**

* @brief Enters the WIZCHIP critical section (weakens global interrupts)

* @note is used to protect the W5500 SPI communication process from interruption, ensuring data integrity.

* Achieved by disabling global interrupts (PRIMASK = 1).

* @Parameters are none

* @retval is none

*/
void SPI_CrisEnter(void);

/**

* @brief Exits the WIZCHIP critical section (restores global boundary).

* @note Used in pair with SPI_CrisEnter; must be called after communication ends to restore interrupt response capability.

* @None parameter

* @retval None

*/
void SPI_CrisExit(void);

/**

* @brief Selects the W5500 chip (pulls the CS chip select pin low)

* @note This function must be called before performing any SPI read/write operations to put the W5500 into the selected state.

* The CS pin is active low.

* @param None

* @retval None

*/
void SPI_CS_Select(void);

/**

* @brief Deselects the W5500 chip (pulls the CS chip select pin high)

* @note This function must be called after SPI communication is complete to release the bus and avoid affecting other devices.

* @param None

* @retval None

*/
void SPI_CS_Deselect(void);


/**

* @brief Registers the callback functions required for WIZCHIP

* @note This function must be called before wizchip_init() to register with ioLibrary_Driver:

* - Critical section enter/exit functions (interrupt protection)

* - Chip select control functions

* - SPI single-byte read/write functions

* After registration, all accesses to the W5500 within the library will automatically call these functions.

* @param None

* @retval None

*/
void register_wizchip();

#endif
