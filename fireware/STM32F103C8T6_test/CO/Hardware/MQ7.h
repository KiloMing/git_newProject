#ifndef __MQ7_H
#define __MQ7_H

#include "stm32f10x.h"

#define MQ7_READ_TIMES       10U
#define MQ7_AO_GPIO_CLK      RCC_APB2Periph_GPIOA
#define MQ7_AO_GPIO_PORT     GPIOA
#define MQ7_AO_GPIO_PIN      GPIO_Pin_0
#define MQ7_ADC_CHANNEL      ADC_Channel_0

extern float MQ7_CO_PPM;

void MQ7_Init(void);
uint16_t MQ7_ReadRaw(void);
float MQ7_Read(void);

#endif
