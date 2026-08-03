#include "./BSP/ADC/adc_1.h"


volatile uint8_t g_adc_dma_finish_flag = 0;

ADC_HandleTypeDef adc_single_sample_dma_handle = {0};

static volatile uint32_t g_adc_dma_error = ADC_DMA_ERROR_NONE;
static uint16_t g_adc_last_value = 0;

static HAL_StatusTypeDef adc_dma_clock_config(void)
{
    RCC_PeriphCLKInitTypeDef periph_clock_config = {0};

    /*
     * HSE = 25 MHz, PLL2 input = 25 / 5 = 5 MHz,
     * PLL2 VCO = 5 * 128 = 640 MHz, PLL2P = 640 / 10 = 64 MHz,
     * ADC asynchronous clock = PLL2P / 2 = 32 MHz.
     */
    periph_clock_config.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    periph_clock_config.AdcClockSelection = RCC_ADCCLKSOURCE_PLL2;
    periph_clock_config.PLL2.PLL2M = 5;
    periph_clock_config.PLL2.PLL2N = 128;
    periph_clock_config.PLL2.PLL2P = 10;
    periph_clock_config.PLL2.PLL2Q = 10;
    periph_clock_config.PLL2.PLL2R = 10;
    periph_clock_config.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
    periph_clock_config.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
    periph_clock_config.PLL2.PLL2FRACN = 0;

    return HAL_RCCEx_PeriphCLKConfig(&periph_clock_config);
}

static HAL_StatusTypeDef adc_dma_start(void)
{
    SCB_InvalidateDCache_by_Addr((uint32_t *)dma_adc_buffer,
                                 ADC_DMA_BUFFER_LENGTH * sizeof(uint16_t));

    if (HAL_ADC_Start_DMA(&adc_single_sample_dma_handle,
                          (uint32_t *)dma_adc_buffer,
                          ADC_DMA_BUFFER_LENGTH) != HAL_OK)
    {
        g_adc_dma_error |= ADC_DMA_ERROR_START;
        return HAL_ERROR;
    }

    return HAL_OK;
}

static HAL_StatusTypeDef adc_dma_restart(void)
{
    if (HAL_ADC_Stop_DMA(&adc_single_sample_dma_handle) != HAL_OK)
    {
        g_adc_dma_error |= ADC_DMA_ERROR_STOP;
        return HAL_ERROR;
    }

    return adc_dma_start();
}

void adc_single_sample_dma_init(void){
    ADC_ChannelConfTypeDef adc_single_sample_dma_channel_config = {0};

    g_adc_dma_finish_flag = 0;
    g_adc_dma_error = ADC_DMA_ERROR_NONE;
    g_adc_last_value = 0;

    if (adc_dma_clock_config() != HAL_OK)
    {
        g_adc_dma_error |= ADC_DMA_ERROR_CLOCK_CONFIG;
        return;
    }

    adc_single_sample_dma_handle.Instance = ADC_SINGLE_SAMPLE_DMA_ADCx;
    adc_single_sample_dma_handle.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV2;
    adc_single_sample_dma_handle.Init.ContinuousConvMode = ENABLE;
    adc_single_sample_dma_handle.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_ONESHOT;
    adc_single_sample_dma_handle.Init.DiscontinuousConvMode = DISABLE;
    adc_single_sample_dma_handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    adc_single_sample_dma_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    adc_single_sample_dma_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    adc_single_sample_dma_handle.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
    adc_single_sample_dma_handle.Init.LowPowerAutoWait = DISABLE;
    adc_single_sample_dma_handle.Init.NbrOfConversion = 3;
    adc_single_sample_dma_handle.Init.NbrOfDiscConversion = 0;
    adc_single_sample_dma_handle.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
    adc_single_sample_dma_handle.Init.OversamplingMode = DISABLE;
    adc_single_sample_dma_handle.Init.Resolution = ADC_RESOLUTION_16B;
    adc_single_sample_dma_handle.Init.ScanConvMode = ADC_SCAN_ENABLE;  //

    if (HAL_ADC_Init(&adc_single_sample_dma_handle) != HAL_OK)
    {
        g_adc_dma_error |= ADC_DMA_ERROR_ADC_INIT;
        return;
    }

    adc_single_sample_dma_channel_config.Channel = ADC_SINGLE_SAMPLE_DMA_CHANNEL;
    adc_single_sample_dma_channel_config.Offset = 0;
    adc_single_sample_dma_channel_config.OffsetNumber = ADC_OFFSET_NONE;
    adc_single_sample_dma_channel_config.OffsetRightShift = DISABLE;
    adc_single_sample_dma_channel_config.OffsetSignedSaturation = DISABLE;
    adc_single_sample_dma_channel_config.Rank = ADC_REGULAR_RANK_1;
    adc_single_sample_dma_channel_config.SamplingTime = ADC_SAMPLETIME_8CYCLES_5;
    adc_single_sample_dma_channel_config.SingleDiff = ADC_SINGLE_ENDED;

    if (HAL_ADC_ConfigChannel(&adc_single_sample_dma_handle,
                              &adc_single_sample_dma_channel_config) != HAL_OK)
    {
        g_adc_dma_error |= ADC_DMA_ERROR_CHANNEL_CONFIG;
        return;
    }

    adc_single_sample_dma_channel_config.Channel = ADC_CHANNEL_14;
    adc_single_sample_dma_channel_config.Rank = ADC_REGULAR_RANK_2;

    if (HAL_ADC_ConfigChannel(&adc_single_sample_dma_handle,
                              &adc_single_sample_dma_channel_config) != HAL_OK)
    {
        g_adc_dma_error |= ADC_DMA_ERROR_CHANNEL_CONFIG;
        return;
    }

    adc_single_sample_dma_channel_config.Channel = ADC_CHANNEL_15;
    adc_single_sample_dma_channel_config.Rank = ADC_REGULAR_RANK_3;

    if (HAL_ADC_ConfigChannel(&adc_single_sample_dma_handle,
                              &adc_single_sample_dma_channel_config) != HAL_OK)
    {
        g_adc_dma_error |= ADC_DMA_ERROR_CHANNEL_CONFIG;
        return;
    }



    if (HAL_ADCEx_Calibration_Start(&adc_single_sample_dma_handle,
                                    ADC_CALIB_OFFSET_LINEARITY,
                                    ADC_SINGLE_ENDED) != HAL_OK)
    {
        g_adc_dma_error |= ADC_DMA_ERROR_CALIBRATION;
        return;
    }

    if (dma_adc_init() != HAL_OK)
    {
        g_adc_dma_error |= ADC_DMA_ERROR_DMA_INIT;
        return;
    }

    __HAL_LINKDMA(&adc_single_sample_dma_handle, DMA_Handle, dma_adc_handle);

    (void)adc_dma_start();
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc){
    GPIO_InitTypeDef gpio_init_struct = {0};

    ADC_SINGLE_SAMPLE_DMA_CLK_ENABLE();
    ADC_SINGLE_SAMPLE_DMA_GPIO_ENABLE();

    gpio_init_struct.Pin = ADC_SINGLE_SAMPLE_DMA_PIN;
    gpio_init_struct.Mode = GPIO_MODE_ANALOG;
    gpio_init_struct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(ADC_SINGLE_SAMPLE_DMA_PORT, &gpio_init_struct);

    gpio_init_struct.Pin = GPIO_PIN_2;
    HAL_GPIO_Init(ADC_SINGLE_SAMPLE_DMA_PORT, &gpio_init_struct);

    gpio_init_struct.Pin = GPIO_PIN_3;
    HAL_GPIO_Init(ADC_SINGLE_SAMPLE_DMA_PORT, &gpio_init_struct);

}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc){
    if(hadc->Instance == ADC_SINGLE_SAMPLE_DMA_ADCx){
        g_adc_dma_finish_flag = 1;
    }
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    if (hadc->Instance == ADC_SINGLE_SAMPLE_DMA_ADCx)
    {
        g_adc_dma_error |= ADC_DMA_ERROR_RUNTIME;
    }
}

uint16_t adc_get_value(void){
    uint32_t temp_sum = 0;

    if(g_adc_dma_finish_flag == 1){
        g_adc_dma_finish_flag = 0;

        SCB_InvalidateDCache_by_Addr((uint32_t *)dma_adc_buffer,
                                     ADC_DMA_BUFFER_LENGTH * sizeof(uint16_t));

        for(uint8_t i = 0; i < ADC_DMA_BUFFER_LENGTH; i++){
            temp_sum += dma_adc_buffer[i];
        }

        g_adc_last_value = (uint16_t)(temp_sum / ADC_DMA_BUFFER_LENGTH);

        if (adc_dma_restart() != HAL_OK)
        {
            g_adc_dma_error |= ADC_DMA_ERROR_RUNTIME;
        }
    }

    return g_adc_last_value;
}

uint32_t adc_get_error(void)
{
    return g_adc_dma_error;
}
