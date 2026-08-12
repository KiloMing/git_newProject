#include "MQ7.h"
#include <math.h>

#define MQ7_ADC_FULL_SCALE 4095.0f
#define MQ7_ADC_VOLTAGE    3.3f
#define MQ7_LOAD_FACTOR    0.5f
#define MQ7_R0             6.64f
#define MQ7_MAX_PPM        500.0f
#define MQ7_FILTER_ALPHA   0.2f
#define MQ7_ADC_CONVERSION_TIMEOUT 100000U

float MQ7_CO_PPM = 0.0f;
static float mq7_filtered_adc = 0.0f;

void MQ7_Init(void)
{
    GPIO_InitTypeDef gpio_init;
    ADC_InitTypeDef adc_init;
    uint32_t timeout;

    RCC_APB2PeriphClockCmd(MQ7_AO_GPIO_CLK | RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    gpio_init.GPIO_Pin = MQ7_AO_GPIO_PIN;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(MQ7_AO_GPIO_PORT, &gpio_init);

    ADC_DeInit(ADC1);
    ADC_StructInit(&adc_init);
    adc_init.ADC_Mode = ADC_Mode_Independent;
    adc_init.ADC_ScanConvMode = DISABLE;
    adc_init.ADC_ContinuousConvMode = DISABLE;
    adc_init.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adc_init.ADC_DataAlign = ADC_DataAlign_Right;
    adc_init.ADC_NbrOfChannel = 1U;
    ADC_Init(ADC1, &adc_init);

    ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);
    timeout = MQ7_ADC_CONVERSION_TIMEOUT;
    while ((ADC_GetResetCalibrationStatus(ADC1) == SET) && (timeout > 0U))
    {
        timeout--;
    }
    ADC_StartCalibration(ADC1);
    timeout = MQ7_ADC_CONVERSION_TIMEOUT;
    while ((ADC_GetCalibrationStatus(ADC1) == SET) && (timeout > 0U))
    {
        timeout--;
    }
}

uint16_t MQ7_ReadRaw(void)
{
    uint32_t timeout = MQ7_ADC_CONVERSION_TIMEOUT;

    ADC_RegularChannelConfig(ADC1, MQ7_ADC_CHANNEL, 1U, ADC_SampleTime_55Cycles5);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while ((ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET) && (timeout > 0U))
    {
        timeout--;
    }

    if (timeout == 0U)
    {
        return 0U;
    }

    return ADC_GetConversionValue(ADC1);
}

float MQ7_Read(void)
{
    uint8_t index;
    uint8_t valid_samples = 0U;
    uint32_t sum = 0U;
    uint16_t sample;
    uint16_t minimum = 0xFFFFU;
    uint16_t maximum = 0U;
    float adc_average;
    float voltage;
    float sensor_resistance;

    for (index = 0U; index < MQ7_READ_TIMES; index++)
    {
        sample = MQ7_ReadRaw();
        if (sample > 0U)
        {
            sum += sample;
            valid_samples++;
            if (sample < minimum)
            {
                minimum = sample;
            }
            if (sample > maximum)
            {
                maximum = sample;
            }
        }
    }

    if (valid_samples == 0U)
    {
        return MQ7_CO_PPM;
    }

    if (valid_samples > 2U)
    {
        sum -= minimum;
        sum -= maximum;
        valid_samples -= 2U;
    }

    adc_average = (float)sum / (float)valid_samples;
    if (mq7_filtered_adc == 0.0f)
    {
        mq7_filtered_adc = adc_average;
    }
    else
    {
        mq7_filtered_adc += MQ7_FILTER_ALPHA * (adc_average - mq7_filtered_adc);
    }

    if (mq7_filtered_adc >= (MQ7_ADC_FULL_SCALE - 1.0f))
    {
        MQ7_CO_PPM = MQ7_MAX_PPM;
        return MQ7_CO_PPM;
    }

    adc_average = mq7_filtered_adc;
    voltage = adc_average * MQ7_ADC_VOLTAGE / MQ7_ADC_FULL_SCALE;
    sensor_resistance = (MQ7_ADC_VOLTAGE - voltage) / (voltage * MQ7_LOAD_FACTOR);
    MQ7_CO_PPM = (float)pow(11.5428f * MQ7_R0 / sensor_resistance, 0.6549f);

    if (MQ7_CO_PPM > MQ7_MAX_PPM)
    {
        MQ7_CO_PPM = MQ7_MAX_PPM;
    }

    return MQ7_CO_PPM;
}
