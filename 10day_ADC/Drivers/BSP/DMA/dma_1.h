#ifndef __DMA_1_H
#define __DMA_1_H

#include "./SYSTEM/sys/sys.h"

#define MAX_SIZES       16
extern volatile uint8_t adc_dma_finish_flag;
extern uint16_t adc_dma_buff[MAX_SIZES];
extern DMA_HandleTypeDef adc_dma_1_handle;

void adc_dma_1_init(void);



#endif
