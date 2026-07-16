#include "adc_1.h"

ADC_HandleTypeDef adc_1_init_handle = {0};

void adc_1_init(void)
{

    ADC_ChannelConfTypeDef adc_1_channel = {0};

    adc_1_init_handle.Instance = ADC1;
    adc_1_init_handle.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV2;
    adc_1_init_handle.Init.ContinuousConvMode = ENABLE;
    adc_1_init_handle.Init.DiscontinuousConvMode = DISABLE;
    adc_1_init_handle.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR;   //使用DMA转运
    adc_1_init_handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    adc_1_init_handle.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
    adc_1_init_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    adc_1_init_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    adc_1_init_handle.Init.LowPowerAutoWait = DISABLE;
    adc_1_init_handle.Init.NbrOfConversion = 1;
    adc_1_init_handle.Init.NbrOfDiscConversion = 1;
    adc_1_init_handle.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    //adc_1_init_handle.Init.Oversampling = ;
    adc_1_init_handle.Init.OversamplingMode = DISABLE;
    adc_1_init_handle.Init.Resolution = ADC_RESOLUTION_12B;
    adc_1_init_handle.Init.ScanConvMode = ADC_SCAN_DISABLE;
    HAL_ADC_Init(&adc_1_init_handle);

    HAL_ADCEx_Calibration_Start(&adc_1_init_handle, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED);

    adc_1_channel.Channel = ADC_CH;
    adc_1_channel.Rank = ADC_REGULAR_RANK_1;
    adc_1_channel.Offset = 0;
    adc_1_channel.SamplingTime = ADC_SAMPLETIME_64CYCLES_5;
    adc_1_channel.SingleDiff = ADC_SINGLE_ENDED;

    HAL_ADC_ConfigChannel(&adc_1_init_handle, &adc_1_channel);



}

 void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    GPIO_InitTypeDef gpio_init_structure = {0};
    if(hadc->Instance == ADC1){
        __HAL_RCC_ADC12_CLK_ENABLE();
        ADC_1_PORT_ENABLE();

        __HAL_RCC_ADC_CONFIG(RCC_ADCCLKSOURCE_CLKP);

        gpio_init_structure.Mode = GPIO_MODE_ANALOG;
        gpio_init_structure.Pin = ADC_1_PIN;
        gpio_init_structure.Pull = GPIO_NOPULL;
        gpio_init_structure.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(ADC_1_PORT, &gpio_init_structure);
        adc_dma_1_init();
        __HAL_LINKDMA(hadc, DMA_Handle, adc_dma_1_handle);

        
    }
}

uint16_t adc_1_get_value(void){
    uint16_t value = 0;
    HAL_ADC_Start(&adc_1_init_handle);
    if (HAL_ADC_PollForConversion(&adc_1_init_handle, 10) == HAL_OK)
    {
        value = HAL_ADC_GetValue(&adc_1_init_handle);
    }

    HAL_ADC_Stop(&adc_1_init_handle);

    return value;
}

void adc_1_get_voltage(void){
    uint16_t value = 0;
    char str[20];
    for(int i = 0; i < 10; i++){
        value += adc_1_get_value();
    }
    value = value/10;
    value = value * 3300/4095;

    snprintf(str, sizeof(str), "VLO:%4lumv", value);
    lcd_show_string(50, 40, 200, 32, 32, str, RED);

}

void adc_dma_1_start(void){
    adc_dma_finish_flag = 0;
    HAL_ADC_Start_DMA(&adc_1_init_handle, (uint32_t *)adc_dma_buff, MAX_SIZES);
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC1)
    {
        adc_dma_finish_flag = 1;
    }
}

void adc_1_lcd_voltage_dma(void){
    uint16_t value = 0;
    char str[20];
    if(adc_dma_finish_flag == 1){
        adc_dma_finish_flag = 0;
        for(int i = 0; i < MAX_SIZES; i++){
            value += adc_dma_buff[i];
        }
        value = value / MAX_SIZES;
        value = value * 3300/4095;
        snprintf(str, sizeof(str), "VOL:%4lumv", value);
        lcd_show_string(50, 40, 200, 32, 32, str, RED);
        //adc_dma_1_start();
        
    }
}



