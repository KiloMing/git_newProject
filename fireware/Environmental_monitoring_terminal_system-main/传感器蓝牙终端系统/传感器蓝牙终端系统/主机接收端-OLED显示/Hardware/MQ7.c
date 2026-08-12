#include "MQ7.h"
#include "BlueTeeth.h"
uint16_t MQ7_Judge_Value = 0;
void MQ7_Init(void)
{

	{
		GPIO_InitTypeDef GPIO_InitStructure;
		
		RCC_APB2PeriphClockCmd (MQ7_AO_GPIO_CLK, ENABLE );	// 打开ADC IO端时钟
		GPIO_InitStructure.GPIO_Pin = MQ7_AO_GPIO_PIN;		// 配置ADC IO端引脚
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		// 设置为模拟输入
		
		GPIO_Init(MQ7_AO_GPIO_PORT, &GPIO_InitStructure);	//初始化 ADC IO

		ADCx_Init();
	}
}


uint16_t MQ7_ADC_Read(void)
{
	//指定ADC的规则组通道，采样时间
	return ADC_GetValue(ADC_CHANNEL, ADC_SampleTime_55Cycles5);
}

//MQ7采集数据
uint16_t MQ7_GetData(void)
{
	
	uint16_t  tempData = 0;
	for (uint8_t i = 0; i < MQ7_READ_TIMES; i++)
	{
		tempData += MQ7_ADC_Read();
		Delay_ms(5);
	}

	tempData /= MQ7_READ_TIMES;
	return tempData;
	
}

// 将MQ7采集到的数据进行计算得到 ppm
float MQ7_GetData_PPM(void)
{

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
}

//本地端OLED显示MQ7采集结果
void MQ7_Result(void){

    OLED_ShowString(1, 1, "CO:");
    OLED_ShowNum(1, 5, MQ7_GetData(), 4);
    char buff[50];
    sprintf((char*)buff, "%.2fppm    ",MQ7_GetData_PPM());
    OLED_ShowString(2, 1, buff);

}


//蓝牙端OLED显示MQ7采集结果
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

