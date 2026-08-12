#ifndef __MQ7_H
#define	__MQ7_H
#include "stm32f10x.h"
#include "ADCx.h"
#include "Delay.h"
#include "OLED.h"
#include "math.h"
#include "string.h"


#define MQ7_READ_TIMES	10  //MQ-7传感器ADC循环读取次数

//模式选择	
//模拟AO:	1
//数字DO:	0
#define	MODE 	1

/***************根据自己需求更改****************/
// MQ-7 GPIO宏定义
#if MODE
#define		MQ7_AO_GPIO_CLK								RCC_APB2Periph_GPIOA
#define 	MQ7_AO_GPIO_PORT							GPIOA
#define 	MQ7_AO_GPIO_PIN								GPIO_Pin_0
#define     ADC_CHANNEL               		            ADC_Channel_0	// ADC 通道宏定义

#else
#define		MQ7_DO_GPIO_CLK								RCC_APB2Periph_GPIOA
#define 	MQ7_DO_GPIO_PORT							GPIOA
#define 	MQ7_DO_GPIO_PIN								GPIO_Pin_1			

#endif
/*********************END**********************/

extern uint16_t MQ7_Judge_Value;
void MQ7_Init(void);
uint16_t MQ7_GetData(void);
float MQ7_GetData_PPM(void);
void MQ7_Result(void);
void MQ7_BlueTeeth_Sent(void);
void MQ7_BlueTeeth_ShowResult(void);
#endif /* __ADC_H */

