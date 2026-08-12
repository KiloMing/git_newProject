/**
 ****************************************************************************************************
 * @file        usart.h
 * @version     V1.1
 * @date        2023-03-02
 * @brief       串口初始化代码(一般是串口1)，支持printf
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:STM32开发板
 *
 * 修改说明
 * V1.0 20220420
 * 第一次发布
 * V1.1 20230607
 * 修改SYS_SUPPORT_OS部分代码, 包含头文件改成:"os.h"
 * 删除USART_UX_IRQHandler()函数的超时处理和修改HAL_UART_RxCpltCallback()
 *
 ****************************************************************************************************
 */

#ifndef _USART_H
#define _USART_H

#include "stdio.h"
#include "./SYSTEM/sys/sys.h"


/*******************************************************************************************************/
/* 引脚和串口 定义 */

#define USART_TX_GPIO_PORT              GPIOA
#define USART_TX_GPIO_PIN               GPIO_PIN_9
#define USART_TX_GPIO_AF                GPIO_AF7_USART1
#define USART_TX_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)   /* 发送引脚时钟使能 */

#define USART_RX_GPIO_PORT              GPIOA
#define USART_RX_GPIO_PIN               GPIO_PIN_10
#define USART_RX_GPIO_AF                GPIO_AF7_USART1
#define USART_RX_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)   /* 接收引脚时钟使能 */

#define USART_UX                        USART1
#define USART_UX_IRQn                   USART1_IRQn
#define USART_UX_IRQHandler             USART1_IRQHandler
#define USART_UX_CLK_ENABLE()           do{ __HAL_RCC_USART1_CLK_ENABLE(); }while(0)  /* USART1 时钟使能 */

#define USART_REC_LEN   200                     /* 定义最大接收字节数 200 */
#define USART_EN_RX     1                       /* 使能（1）/禁止（0）串口1接收 */
#define RXBUFFERSIZE    1                       /* 缓存大小 */

#define UART1_RX_DMA_BUFFER_SIZE    256U        /* DMA Buffer MAX*/

extern UART_HandleTypeDef g_uart1_handle;       /* UART句柄 */
/* 本次接收的长度 */
extern volatile uint16_t g_uart1_rx_length;
/* 接收完成标志位 */
extern volatile uint8_t g_uart1_rx_ready;
extern uint8_t  g_usart_rx_buf[USART_REC_LEN];  /* 接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 */
extern uint16_t g_usart_rx_sta;                 /* 接收状态标记 */
extern uint8_t g_rx_buffer[RXBUFFERSIZE];       /* HAL库USART接收Buffer */


extern uint8_t g_uart1_rx_dma_buffer[UART1_RX_DMA_BUFFER_SIZE]; /* DMA接收缓冲区*/
extern uint8_t g_uart1_rx_frame[UART1_RX_DMA_BUFFER_SIZE + 1U]; /* 数据读取区，额外一字节用于字符串结束符 */
/*******************************************************************************************************/

void usart_init(uint32_t baudrate);             /* 串口初始化函数 */
HAL_StatusTypeDef dma_uart_receice_data(void);
#endif



