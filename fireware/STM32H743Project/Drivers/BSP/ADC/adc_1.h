#ifndef __ADC_1_H
#define __ADC_1_H

#include "./SYSTEM/sys/sys.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/DMA/dma_1.h"
#include <stdlib.h>

extern ADC_HandleTypeDef adc_1_init_handle;

#define ADCx                    ADC1
#define ADC_CH                  ADC_CHANNEL_19

#define ADC_1_PORT              GPIOA
#define ADC_1_PIN               GPIO_PIN_5
#define ADC_1_PORT_ENABLE()     do { __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)
void adc_1_init(void);
uint16_t adc_1_get_value(void);
void adc_1_get_voltage(void);
void adc_1_lcd_voltage_dma(void);
void adc_dma_1_start(void);
#endif

