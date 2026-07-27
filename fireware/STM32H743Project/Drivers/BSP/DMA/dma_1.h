#ifndef __DMA_1_H
#define __DMA_1_H

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#define MAX_SIZES       16

extern volatile uint16_t dma_adc_buffer[MAX_SIZES];
extern DMA_HandleTypeDef dma_adc_handle;
extern DMA_HandleTypeDef g_dma_uart_rx_handle;

HAL_StatusTypeDef dma_adc_init(void);
void dma_uart_rx_init(void);



#endif
