#ifndef __ADCX_H
#define __ADCX_H


#define RCC_APB2Periph_ADCx                     RCC_APB2Periph_ADC1       //开启ADC时钟
#define RCC_APB2Periph_GPIOx                    RCC_APB2Periph_GPIOA      //开启GPIO时钟
#define GPIOx                                   GPIOA                     //选择GPIO
#define ADCx                                    ADC1                      //选择ADC
void ADCx_Init(void);
uint16_t ADC_GetValue(uint8_t ADC_Channel,uint8_t ADC_SampleTime);

#endif