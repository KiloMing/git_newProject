#include "dma_1.h"

volatile uint8_t adc_dma_finish_flag = 0;
uint16_t adc_dma_buff[MAX_SIZES] = {0};

DMA_HandleTypeDef adc_dma_1_handle = {0};

void adc_dma_1_init(void){
    __HAL_RCC_DMA1_CLK_ENABLE();
    adc_dma_1_handle.Instance = DMA1_Stream1;
    adc_dma_1_handle.Init.Direction = DMA_PERIPH_TO_MEMORY;
    adc_dma_1_handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    //adc_dma_1_handle.Init.FIFOThreshold = ;
    adc_dma_1_handle.Init.MemBurst = DMA_MBURST_SINGLE;
    adc_dma_1_handle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    adc_dma_1_handle.Init.MemInc = DMA_MINC_ENABLE;
    adc_dma_1_handle.Init.Mode = DMA_CIRCULAR;
    adc_dma_1_handle.Init.PeriphBurst = DMA_PBURST_SINGLE;
    adc_dma_1_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    adc_dma_1_handle.Init.PeriphInc = DMA_PINC_DISABLE;
    adc_dma_1_handle.Init.Priority = DMA_PRIORITY_MEDIUM;
    adc_dma_1_handle.Init.Request = DMA_REQUEST_ADC1;

    HAL_DMA_Init(&adc_dma_1_handle);

    HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
}

void DMA1_Stream1_IRQHandler(void){
    HAL_DMA_IRQHandler(&adc_dma_1_handle);
}











