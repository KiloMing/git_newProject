#include "MQ7.h"
#include "BlueTeeth.h"
uint16_t MQ7_Judge_Value = 0;
void MQ7_Init(void)
{
	#if MODE
	{
		GPIO_InitTypeDef GPIO_InitStructure;
		
		RCC_APB2PeriphClockCmd (MQ7_AO_GPIO_CLK, ENABLE );	// 打开 ADC IO端口时钟
		GPIO_InitStructure.GPIO_Pin = MQ7_AO_GPIO_PIN;					// 配置 ADC IO 引脚模式
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		// 设置为模拟输入
		
		GPIO_Init(MQ7_AO_GPIO_PORT, &GPIO_InitStructure);				// 初始化 ADC IO

		ADCx_Init();
	}
	#else
	{
		GPIO_InitTypeDef GPIO_InitStructure;
		
		RCC_APB2PeriphClockCmd (MQ7_DO_GPIO_CLK, ENABLE );	// 打开连接 传感器DO 的单片机引脚端口时钟
		GPIO_InitStructure.GPIO_Pin = MQ7_DO_GPIO_PIN;			// 配置连接 传感器DO 的单片机引脚模式
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			// 设置为上拉输入
		
		GPIO_Init(MQ7_DO_GPIO_PORT, &GPIO_InitStructure);				// 初始化 
		
	}
	#endif
	
}

#if MODE
uint16_t MQ7_ADC_Read(void)
{
	//设置指定ADC的规则组通道，采样时间
	return ADC_GetValue(ADC_CHANNEL, ADC_SampleTime_55Cycles5);
}
#endif

uint16_t MQ7_GetData(void)
{
	
	#if MODE
	uint16_t  tempData = 0;
	for (uint8_t i = 0; i < MQ7_READ_TIMES; i++)
	{
		tempData += MQ7_ADC_Read();
		Delay_ms(5);
	}

	tempData /= MQ7_READ_TIMES;
	return tempData;
	
	#else
	uint16_t tempData;
	tempData = !GPIO_ReadInputDataBit(MQ7_DO_GPIO_PORT, MQ7_DO_GPIO_PIN);
	return tempData;
	#endif
}


float MQ7_GetData_PPM(void)
{
	#if MODE
	float  tempData = 0;
	

	for (uint8_t i = 0; i < MQ7_READ_TIMES; i++)
	{
		tempData += MQ7_ADC_Read();
		Delay_ms(5);
	}
	tempData /= MQ7_READ_TIMES;
	
	float Vol = (tempData*5/4096);
	float RS = (5-Vol)/(Vol*0.5);
	float R0=6.64;
	
	float ppm = pow(11.5428*R0/RS, 0.6549f);
	
	return ppm;
	#endif
}

void MQ7_Result(void){

    OLED_ShowString(1, 1, "CO:");
    OLED_ShowNum(1, 5, MQ7_GetData(), 4);
    char buff[50];
    sprintf((char*)buff, "%.2fppm    ",MQ7_GetData_PPM());
    OLED_ShowString(2, 1, buff);

}

void MQ7_BlueTeeth_Sent(void){
    BlueTeeth_SentByte(MQ7_GetData());
    BlueTeeth_SentByte(0x00);
}

void MQ7_BlueTeeth_ShowResult(void){
    OLED_ShowString(1, 1, "CO:");
    uint16_t DATA = Result[3];
    DATA <<= 8;
    DATA += Result[4];
    OLED_ShowNum(1, 5, DATA, 4);
    float Vol = ((float)DATA*5/4096);
	float RS = (5-Vol)/(Vol*0.5);
	float R0=6.64;
	float ppm = pow(11.5428*R0/RS, 0.6549f);
    char buff[50];
    sprintf((char*)buff, "%.2fppm    ",ppm);
    OLED_ShowString(2, 1, buff);

}

