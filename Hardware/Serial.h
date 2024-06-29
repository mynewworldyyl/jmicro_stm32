#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdio.h>
#include "jm_buffer.h"

#define JM_UART_PRO_NOMAL 0 //普通数据包，不加任何前缀
#define JM_UART_PRO_NETPCK 1 //JM数据包，前面加一个字节表示接下来的数据长度

#define JM_UART1 1
#define JM_UART2 2
#define JM_UART3 3

//typedef void (*Serial_onBufData)(uint8_t uartNo, jm_buf_t *buf);

typedef void (*Serial_onByteData)(uint8_t uartNo, uint8_t byte);

void Serial_Init(uint8_t uartNo, Serial_onByteData dcb, int8_t proType);

int getRXBuffer(uint8_t *pBuffer, int bufferLen, uint8_t bufIdx);

void Serial_SendByte(uint8_t jmUartNo, uint8_t Byte);
void Serial_SendArray(uint8_t jmUartNo, uint8_t *Array, uint16_t Length);
void Serial_SendString(uint8_t jmUartNo, char *String);
void Serial_SendNumber(uint8_t jmUartNo, uint32_t Number, uint8_t Length);
void Serial_Printf(uint8_t jmUartNo, char *format, ...);

//写整个包+包长度
void Serial_SendJmPckBuf(uint8_t jmUartNo, jm_buf_t *buf);
void Serial_SendJmPckArray(uint8_t jmUartNo, uint8_t *Array, uint16_t Length);

//写包长度
void Serial_writeLen(uint8_t jmUartNo, uint16_t len);
//写入Bug，不包括长度
void Serial_writeBuf(uint8_t jmUartNo, jm_buf_t *buf);

//void Serial_canRecv(BOOL can);

#endif
