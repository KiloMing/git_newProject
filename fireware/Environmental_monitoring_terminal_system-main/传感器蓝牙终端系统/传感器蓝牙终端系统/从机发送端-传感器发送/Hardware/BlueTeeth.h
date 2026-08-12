#ifndef __BLUETEETH_H
#define __BLUETEETH_H
#include "stm32f10x.h"                  // Device header

extern char str[10];
extern uint16_t BlueTeeth_value;
extern uint16_t Result[];
/**************自定义修改**************/
#define RCC_APB2Periph_USARTx       RCC_APB2Periph_USART1
#define BlueTeeth_GPIOx             GPIOA       
#define BlueTeeth_GPIO_CLK          RCC_APB2Periph_GPIOA   
#define BlueTeeth_Pin_Tx            GPIO_Pin_9
#define BlueTeeth_Pin_Rx            GPIO_Pin_10

#define USARTx                      USART1

void BlueTeeth_Init(void);
void BlueTeeth_SentByte(uint8_t Byte);
#endif