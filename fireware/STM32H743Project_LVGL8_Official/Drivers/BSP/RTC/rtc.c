#include "./BSP/RTC/rtc.h"

RTC_HandleTypeDef g_rtc_init_handle = {0};

/**
 * @brief set time 
 * @param hour: hour (0-12)/(0-23)
 * @param min : minute (0-59)
 * @param sec : seconds (0-59)
 * @return void;
 */

void rtc_set_time(uint8_t hour, uint8_t min, uint8_t sec)
{
    RTC_TimeTypeDef rtc_time_handle;

    rtc_time_handle.Hours = hour;
    rtc_time_handle.Minutes = min;
    rtc_time_handle.Seconds = sec;
    rtc_time_handle.TimeFormat = RTC_HOURFORMAT12_PM;
    rtc_time_handle.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    rtc_time_handle.StoreOperation = RTC_STOREOPERATION_RESET;

    HAL_RTC_SetTime(&g_rtc_init_handle, &rtc_time_handle, RTC_FORMAT_BIN);
}

/**
 * @brief set data
 * @param year
 * @param month
 * @param data
 * @param week : weekday
 * @return void
 */
void rtc_set_date(uint8_t year, uint8_t month, uint8_t date, uint8_t week)
{
    RTC_DateTypeDef rtc_date_handle;

    rtc_date_handle.Date = date;
    rtc_date_handle.Month = month;
    rtc_date_handle.WeekDay = week;
    rtc_date_handle.Year = year;

    HAL_RTC_SetDate(&g_rtc_init_handle, &rtc_date_handle, RTC_FORMAT_BIN);
}

/**
 * @brief       get rtc time
 * @param       *hour,*min,*sec 
 * @param       *ampm           : AM/PM,0=AM/24H,1=PM/12H.
 * @retval      void
 */
void rtc_get_time(uint8_t *hour, uint8_t *min, uint8_t *sec, uint8_t *ampm)
{
    
    RTC_TimeTypeDef rtc_time_handle;

    HAL_RTC_GetTime(&g_rtc_init_handle, &rtc_time_handle, RTC_FORMAT_BIN);

    *hour = rtc_time_handle.Hours;
    *min = rtc_time_handle.Minutes;
    *sec = rtc_time_handle.Seconds;
    *ampm = rtc_time_handle.TimeFormat;
}

/**
 * @brief get the data of RTC
 * @param *year
 * @param *month
 * @param *data
 * @param *week weekday 
 * 
 */
void rtc_get_date(uint8_t *year, uint8_t *month, uint8_t *date, uint8_t *week)
{
    RTC_DateTypeDef rtc_date_handle;

    HAL_RTC_GetDate(&g_rtc_init_handle, &rtc_date_handle, RTC_FORMAT_BIN);

    *year = rtc_date_handle.Year;
    *month = rtc_date_handle.Month;
    *date = rtc_date_handle.Date;
    *week = rtc_date_handle.WeekDay;
}



static uint16_t bkpflag = 0;

/**
 * @brief Initlization RTC at time first start of the program.
 */

void rtc_init(void)
{
    g_rtc_init_handle.Instance = RTC;
    g_rtc_init_handle.Init.HourFormat = RTC_HOURFORMAT_24;
    g_rtc_init_handle.Init.OutPut = RTC_OUTPUT_DISABLE;
    g_rtc_init_handle.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    //g_rtc_init_handle.Init.OutPutRemap = ;
    g_rtc_init_handle.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    /*
    LSE = 32.768kHz = 32768 Hz, so we set the Prediv_A = 128, the Prediv_S = 256
    128 * 256 = 32768, the frequence equal to 1 Hz.
    */
    g_rtc_init_handle.Init.SynchPrediv = 256 -1;
    g_rtc_init_handle.Init.AsynchPrediv = 128 - 1;

    if(bkpflag == 1){
        return;
    }
    bkpflag = 1;
    HAL_RTC_Init(&g_rtc_init_handle);
}

void HAL_RTC_MspInit(RTC_HandleTypeDef *hrtc)
{
    

    RCC_OscInitTypeDef rcc_osc_init_handle;
    RCC_PeriphCLKInitTypeDef rcc_periphclk_init_handle;

    __HAL_RCC_RTC_ENABLE();                                                 /* RTC Enable */
    HAL_PWR_EnableBkUpAccess();                                             /* Disable write protection in the backup area */
    __HAL_RCC_RTC_ENABLE();                                                 /* RTC Enable */

    delay_ms(200);
    rcc_osc_init_handle.OscillatorType = RCC_OSCILLATORTYPE_LSE;        /* Select the oscillator to be configured */
    rcc_osc_init_handle.PLL.PLLState = RCC_PLL_NONE;                    /* PLL dosen't be configured */
    rcc_osc_init_handle.LSEState = RCC_LSE_ON;                          /* LSE : Enable */
    HAL_RCC_OscConfig(&rcc_osc_init_handle);                            /* config rcc_oscinitstruct */

    rcc_periphclk_init_handle.PeriphClockSelection = RCC_PERIPHCLK_RTC; /* Select to configure the peripheral RTC */
    rcc_periphclk_init_handle.RTCClockSelection = RCC_RTCCLKSOURCE_LSE; /* Select LSE as the RTC clock source  */
    HAL_RCCEx_PeriphCLKConfig(&rcc_periphclk_init_handle);              /* config rcc_periphclkinitstruct */

}
