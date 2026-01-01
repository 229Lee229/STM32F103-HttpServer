#ifndef __MYDRV_H
#define __MYDRV_H
// #include "wizchip_conf.h"
/**
  * @brief  ?????
  * @retval None
  */
void SPI_CrisEnter(void);
void SPI_CrisExit(void);
void SPI_CS_Select(void);
void SPI_CS_Deselect(void);

void register_wizchip();

#endif
