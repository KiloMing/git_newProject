#include "stm32f10x.h"                  // Device header
#include "ADCx.h"

uint8_t ADC_Value;

void ADCx_Init(void){
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADCx, ENABLE);
    
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
    ADC_InitTypeDef ADC_InitStruct;
    ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;     //单次模式
    ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;     //数据右对齐
    ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; 
    ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;           //独立模式
    ADC_InitStruct.ADC_NbrOfChannel = 1;    //指定通道总数
    ADC_InitStruct.ADC_ScanConvMode = DISABLE;     // 非扫描模式
   
    ADC_Init(ADCx, &ADC_InitStruct);
    
    ADC_Cmd(ADCx, ENABLE);      //开启ADC时钟

	ADC_ResetCalibration(ADCx);
	while(ADC_GetResetCalibrationStatus(ADCx) == SET);
	ADC_StartCalibration(ADCx);
	while(ADC_GetCalibrationStatus(ADCx) == SET);	
}
/**
  * @brief  获取ADC转换后的数据
  * @param  ADC_Channel      选择ADC的采集通道
  * @param  ADC_SampleTime   选择采样时间
  * @retval 返回转换后的模拟信号值



 */
uint16_t ADC_GetValue(uint8_t ADC_Channel,uint8_t ADC_SampleTime)
{
	//配置ADC通道
	ADC_RegularChannelConfig(ADCx, ADC_Channel, 1, ADC_SampleTime);
	
	ADC_SoftwareStartConvCmd(ADCx, ENABLE); //软件触发ADC转换
	while(ADC_GetFlagStatus(ADCx, ADC_FLAG_EOC) == RESET); //读取ADC转换标志位
	return ADC_GetConversionValue(ADCx);
}






