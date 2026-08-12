/**
 ****************************************************************************************************
 * @file        usart.h
 * @version     V1.0
 * @date        2022-09-6
 * @brief       串口初始化代码(一般是串口1)，支持printf
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:STM32H743开发板
 *
 * 修改说明
 * V1.0 20220906
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef _USART_H
#define _USART_H

#include "stdio.h"
#include "./SYSTEM/sys/sys.h"

/*******************************************************************************************************/
/* 引脚 和 串口 定义 
 * 默认是针对USART1的.
 * 注意: 通过修改这12个宏定义,可以支持USART1~UART7任意一个串口.
 */

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

/*******************************************************************************************************/

#define USART_REC_LEN   200                     /* 定义最大接收字节数 200 */
#define USART_EN_RX     1                       /* 使能（1）/禁止（0）串口1接收 */
#define RXBUFFERSIZE    1                       /* 缓存大小 */
#define UART1_RX_DMA_BUFFER_SIZE    256U        /* USART1 DMA接收缓冲区大小 */
#define UART1_RX_FRAME_QUEUE_DEPTH  4U          /* USART1 DMA接收帧队列深度 */

extern UART_HandleTypeDef g_uart1_handle;       /* UART句柄 */

extern uint8_t  g_usart_rx_buf[USART_REC_LEN];  /* 接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 */
extern uint16_t g_usart_rx_sta;                 /* 接收状态标记 */
extern uint8_t g_rx_buffer[RXBUFFERSIZE];       /* HAL库USART接收Buffer */
extern uint8_t g_uart1_rx_dma_buffer[UART1_RX_DMA_BUFFER_SIZE];


void usart_init(uint32_t baudrate);             /* 串口初始化函数 */
HAL_StatusTypeDef dma_uart_receice_data(void);  /* 启动USART1空闲线DMA接收 */
void uart1_rx_frame_queue_reset(void);          /* 清空USART1 DMA接收帧队列 */
uint16_t uart1_rx_frame_take(uint8_t *data, uint16_t capacity); /* 按接收顺序取出一帧 */

#endif







