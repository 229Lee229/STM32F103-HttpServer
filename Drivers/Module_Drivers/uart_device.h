// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (c) 2008-2023 100askTeam : Dongshan WEI <weidongshan@qq.com> 
 * Discourse:  https://forums.100ask.net
 */
 
/*  Copyright (C) 2008-2023 深圳百问网科技有限公司
 *  All rights reserved
 *
 * 免责声明: 百问网编写的文档, 仅供学员学习使用, 可以转发或引用(请保留作者信息),禁止用于商业用途！
 * 免责声明: 百问网编写的程序, 可以用于商业用途, 但百问网不承担任何后果！
 * 
 * 本程序遵循GPL V3协议, 请遵循协议
 * 百问网学习平台   : https://www.100ask.net
 * 百问网交流社区   : https://forums.100ask.net
 * 百问网官方B站    : https://space.bilibili.com/275908810
 * 本程序所用开发板 : DShanMCU-F103
 * 百问网官方淘宝   : https://100ask.taobao.com
 * 联系我们(E-mail): weidongshan@qq.com
 *
 *          版权所有，盗版必究。
 *  
 * 修改历史     版本号           作者        修改内容
 *-----------------------------------------------------
 * 2024.02.01      v01         百问科技      创建文件
 *-----------------------------------------------------
 */
#ifndef __UART_DEVICE_H
#define __UART_DEVICE_H

#include <stdint.h>
struct UART_Device {
    char *name;
	int (*Init)( struct UART_Device *pDev, int baud, char parity, int data_bit, int stop_bit);
	int (*Send)( struct UART_Device *pDev, uint8_t *datas, uint32_t len, int timeout);
	int (*RecvByte)( struct UART_Device *pDev, uint8_t *data, int timeout);
	int (*Flush)(struct UART_Device *pDev);
    void *priv_data;
}; // UART_Device, *PUART_Device;


struct UART_Device *GetUARTDevice(char *name);

    
#endif /* __UART_DEVICE_H */

