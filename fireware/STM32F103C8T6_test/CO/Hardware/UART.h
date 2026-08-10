#ifndef __UART_H
#define __UART_H

#include "stm32f10x.h"

/* USART1: TX = PA9, RX = PA10. */
void UART_Init(uint32_t BaudRate);

/* Send the byte value directly on the wire. */
void UART_SendByte(uint8_t Byte);
void UART_SendArray(const uint8_t *Data, uint16_t Length);
void UART_SendString(const char *String);

/* Send an uppercase ASCII hexadecimal representation. */
void UART_SendHexByte(uint8_t Byte);
void UART_SendHex(uint32_t Number, uint8_t Length);
void UART_SendHexArray(const uint8_t *Data, uint16_t Length, char Separator);

/* Blocking receive functions. A string is terminated by CR or LF. */
uint8_t UART_ReceiveByte(void);
uint16_t UART_ReceiveString(char *Buffer, uint16_t BufferSize);

/* Signed decimal integer receive and conversion. Return 1 on success. */
uint8_t UART_ParseInt32(const char *String, int32_t *Number);
uint8_t UART_ReceiveInt32(int32_t *Number);

/* USART Send Lora */
void UART_SendLora_num(const uint8_t *Data, uint16_t Length, uint8_t AddressHigh,
                       uint8_t AddressLow, uint8_t Channel);
#endif
