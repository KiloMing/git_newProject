#ifndef __ADC_1_H
#define __ADC_1_H

#include "./SYSTEM/sys/sys.h"
#include "./BSP/DMA/dma_1.h"

extern volatile uint8_t g_adc_dma_finish_flag;

#define ADC_DMA_ERROR_NONE              0x00000000UL
#define ADC_DMA_ERROR_CLOCK_CONFIG      0x00000001UL
#define ADC_DMA_ERROR_ADC_INIT          0x00000002UL
#define ADC_DMA_ERROR_CHANNEL_CONFIG    0x00000004UL
#define ADC_DMA_ERROR_CALIBRATION       0x00000008UL
#define ADC_DMA_ERROR_DMA_INIT          0x00000010UL
#define ADC_DMA_ERROR_START             0x00000020UL
#define ADC_DMA_ERROR_STOP              0x00000040UL
#define ADC_DMA_ERROR_RUNTIME           0x00000080UL

#define ADC_SINGLE_SAMPLE_DMA_PORT      GPIOA
#define ADC_SINGLE_SAMPLE_DMA_PIN       GPIO_PIN_5
#define ADC_SINGLE_SAMPLE_DMA_GPIO_ENABLE()     do { __HAL_RCC_GPIOA_CLK_ENABLE(); } while(0)

#define ADC_SINGLE_SAMPLE_DMA_ADCx      ADC1
#define ADC_SINGLE_SAMPLE_DMA_CHANNEL   ADC_CHANNEL_19
#define ADC_SINGLE_SAMPLE_DMA_CLK_ENABLE()      do { __HAL_RCC_ADC12_CLK_ENABLE(); } while(0)

#define ADC_MUTILE


/* 三个规则通道各转换一次，DMA缓冲区也必须恰好能容纳这三个结果。 */
#define ADC_DMA_BUFFER_LENGTH           3U


uint16_t adc_get_value(void);
uint32_t adc_get_error(void);
void adc_single_sample_dma_init(void);

#endif
