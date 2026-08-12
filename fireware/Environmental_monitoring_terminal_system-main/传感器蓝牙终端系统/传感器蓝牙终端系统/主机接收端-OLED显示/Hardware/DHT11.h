#ifndef __DHT11_H
#define __DHT11_H
#include "stm32f10x.h"                  // Device header

extern uint8_t T1, T2, W1, W2;

/***********根据自己的需求更改************/
#define DHT11_GPIOx						GPIOA
#define DHT11_GPIO_CLK 					RCC_APB2Periph_GPIOA
#define DHT11_GPIO_Pin					GPIO_Pin_2

uint8_t DHT_Init(void);
void DHT11_Result(void);
void SaveData(uint8_t *T, uint8_t *T2, uint8_t *W, uint8_t *W2);
void DHT11_BlueTeeth_Sent(void);
void DHT11_BlueTeeth_ShowResult(void);
#endif