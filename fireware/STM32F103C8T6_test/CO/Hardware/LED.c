#include "stm32f10x.h"                  // Device header

void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	//设置端口模式
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_7 |GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	/* LED为高电平点亮，初始化时输出低电平并保持熄灭。 */
	GPIO_ResetBits(GPIOA,GPIO_Pin_7 |GPIO_Pin_6);
}

void LED1_OFF(void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_7);
	
}

void LED1_ON(void)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_7);
}

void LED2_OFF(void)
{
	GPIO_ResetBits(GPIOA,GPIO_Pin_6);
	
}

void LED2_ON(void)
{
	GPIO_SetBits(GPIOA,GPIO_Pin_6);
}

//电平翻转
void LED1_Turn(void)
{
	if(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_7)==0)
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_7);//如果输入0就置1
	}
	else
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_7);
	}
}


void LED2_Turn(void)
{
	if(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_6)==0)
	{
		GPIO_SetBits(GPIOA,GPIO_Pin_6);//如果输入0就置1
	}
	else
	{
		GPIO_ResetBits(GPIOA,GPIO_Pin_6);
	}
}
