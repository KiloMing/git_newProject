#include "dma_1.h"

ALIGN_32BYTES(volatile uint16_t dma_adc_buffer[MAX_SIZES]) = {0};

DMA_HandleTypeDef dma_adc_handle = {0};

/**
 * @brief Use DMA to collect data from ADC->DR (Init)
 * @param void
 * @return HAL status
 */
HAL_StatusTypeDef dma_adc_init(void){
    __HAL_RCC_DMA1_CLK_ENABLE();
    dma_adc_handle.Instance = DMA1_Stream1;
    dma_adc_handle.Init.Direction = DMA_PERIPH_TO_MEMORY;
    dma_adc_handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    //dma_adc_handle.Init.FIFOThreshold = ;
    dma_adc_handle.Init.MemBurst = DMA_MBURST_SINGLE;
    dma_adc_handle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    dma_adc_handle.Init.MemInc = DMA_MINC_ENABLE;
    dma_adc_handle.Init.Mode = DMA_CIRCULAR;
    dma_adc_handle.Init.PeriphBurst = DMA_PBURST_SINGLE;
    dma_adc_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    dma_adc_handle.Init.PeriphInc = DMA_PINC_DISABLE;
    dma_adc_handle.Init.Priority = DMA_PRIORITY_MEDIUM;
    dma_adc_handle.Init.Request = DMA_REQUEST_ADC1;

    if (HAL_DMA_Init(&dma_adc_handle) != HAL_OK)
    {
        return HAL_ERROR;
    }

    HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);

    return HAL_OK;
}


DMA_HandleTypeDef g_dma_uart_rx_handle = {0};

/**
 * @brief initialize the DMA to use the DMA to transfer data for UAER RX
 */
void dma_uart_rx_init(void)
{
    __HAL_RCC_DMA1_CLK_ENABLE();
    g_dma_uart_rx_handle.Instance = DMA1_Stream2;
    g_dma_uart_rx_handle.Init.Request = DMA_REQUEST_USART1_RX;
    g_dma_uart_rx_handle.Init.PeriphInc = DMA_PINC_DISABLE;
    g_dma_uart_rx_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    g_dma_uart_rx_handle.Init.PeriphBurst = DMA_PBURST_SINGLE;
    g_dma_uart_rx_handle.Init.Mode = DMA_NORMAL;
    g_dma_uart_rx_handle.Init.MemInc = DMA_MINC_ENABLE;
    g_dma_uart_rx_handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    g_dma_uart_rx_handle.Init.MemBurst = DMA_MBURST_SINGLE;
    g_dma_uart_rx_handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    g_dma_uart_rx_handle.Init.Direction = DMA_PERIPH_TO_MEMORY;
    g_dma_uart_rx_handle.Init.Priority = DMA_PRIORITY_MEDIUM;

    HAL_DMA_Init(&g_dma_uart_rx_handle);
    __HAL_LINKDMA(&g_uart1_handle, hdmarx, g_dma_uart_rx_handle);

    HAL_NVIC_SetPriority(DMA1_Stream2_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream2_IRQn);
}

void DMA1_Stream1_IRQHandler(void){
    HAL_DMA_IRQHandler(&dma_adc_handle);
}



void DMA1_Stream2_IRQHandler(void){
    HAL_DMA_IRQHandler(&g_dma_uart_rx_handle);
}

