/**
 ****************************************************************************************************
 * @file        app_runtime.c
 * @brief       RTC与LVGL统一接口之间的应用层刷新逻辑
 *
 * @note
 * 硬件初始化由main()调用；所有LVGL接口只在LVGL FreeRTOS任务中调用，
 * 防止多个任务同时修改LVGL对象。
 ****************************************************************************************************
 */

#include "app_runtime.h"
#include "./SYSTEM/sys/sys.h"
#include "./BSP/RTC/rtc.h"
#include "app_interface.h"

#define APP_UI_UPDATE_PERIOD_MS 1000U

static uint32_t s_last_ui_update_ms;
static uint8_t s_simulation_phase;

volatile uint8_t alarm_flag = 0;

#define ANARM_FANG_SAFE       3
#define ANARM_FANG_WRON       2
#define ANARM_FANG_DANGLE     1
void app_runtime_hardware_init(void)
{
    uint8_t year;
    uint8_t month;
    uint8_t date;
    uint8_t week;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t ampm;

    rtc_init();

    /* STM32 RTC要求先读取Time再读取Date，以解除影子寄存器锁定。 */
    rtc_get_time(&hour, &minute, &second, &ampm);
    rtc_get_date(&year, &month, &date, &week);

    /* 仅在后备域数据无效时设置初值，正常复位不会重置正在运行的RTC。 */
    if(month < 1U || month > 12U || date < 1U || date > 31U ||
       hour > 23U || minute > 59U || second > 59U) {
        rtc_set_date(26U, 8U, 3U, RTC_WEEKDAY_MONDAY);
        rtc_set_time(12U, 0U, 0U);
    }
}

/** 从RTC读取时间，并刷新所有实时LVGL组件。 */
static void app_runtime_refresh_widgets(void)
{
    uint8_t year;
    uint8_t month;
    uint8_t date;
    uint8_t week;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t ampm;
    float gas_values[APP_SENSOR_COUNT];
    float triangle;
    float o2_offset;

    rtc_get_time(&hour, &minute, &second, &ampm);
    rtc_get_date(&year, &month, &date, &week);
    app_interface_set_datetime((uint16_t)(2000U + year), month, date, hour, minute);

    /* triangle每秒从0上升到60，再下降到0，用于生成可重复的测试数据。 */
    triangle = (s_simulation_phase <= 60U)
               ? (float)s_simulation_phase
               : (float)(120U - s_simulation_phase);

    /* 每个数组下标严格使用app_sensor_id_t，防止气体名称和数值错位。 */
    gas_values[APP_SENSOR_CO]  = triangle * 0.60f;          /* 0~36 ppm */
    gas_values[APP_SENSOR_CO2] = 400.0f + triangle * 80.0f; /* 400~5200 ppm */
    gas_values[APP_SENSOR_H2S] = triangle * 0.20f;          /* 0~12 ppm */
    gas_values[APP_SENSOR_SO2] = triangle * 0.04f;          /* 0~2.4 ppm */
    gas_values[APP_SENSOR_NH3] = triangle * 0.50f;          /* 0~30 ppm */

    /* O2围绕20.9%上下变化，依次经过高氧和低氧阈值。 */
    if(s_simulation_phase <= 30U) {
        o2_offset = (float)s_simulation_phase;
    }
    else if(s_simulation_phase <= 90U) {
        o2_offset = (float)(60 - (int32_t)s_simulation_phase);
    }
    else {
        o2_offset = (float)((int32_t)s_simulation_phase - 120);
    }
    gas_values[APP_SENSOR_O2] = 20.9f + o2_offset * 0.10f; /* 17.9~23.9 %vol */

    app_interface_set_all_gas_values(gas_values);
    app_interface_set_environment(22.0f + triangle * 0.10f,  /* 22.0~28.0 C */
                                  45.0f + triangle * 0.40f); /* 45.0~69.0 %RH */

    s_simulation_phase++;
    if(s_simulation_phase > 120U) {
        s_simulation_phase = 0U;
    }
}

void app_runtime_ui_start(void)
{
    /* 用户要求CO和CO2显示为已连接，其他四路保持未连接。 */
    app_interface_set_sensor_connected(APP_SENSOR_CO, true);
    app_interface_set_sensor_connected(APP_SENSOR_CO2, true);
    app_interface_set_sensor_connected(APP_SENSOR_O2, false);
    app_interface_set_sensor_connected(APP_SENSOR_H2S, false);
    app_interface_set_sensor_connected(APP_SENSOR_SO2, false);
    app_interface_set_sensor_connected(APP_SENSOR_NH3, false);

    s_simulation_phase = 0U;
    s_last_ui_update_ms = HAL_GetTick();
    app_runtime_refresh_widgets();
}

void app_runtime_ui_process(void)
{
    uint32_t now = HAL_GetTick();
    // 每秒刷新一次界面，避免LVGL任务占用过多CPU时间。
    if((uint32_t)(now - s_last_ui_update_ms) >= APP_UI_UPDATE_PERIOD_MS) {
        s_last_ui_update_ms = now;
        app_runtime_refresh_widgets();
    }
}



void app_runtime_analyze(void)
{
    if(g_test_array[APP_SENSOR_CO].test_value >= 65.0f ||
       g_test_array[APP_SENSOR_CO2].test_value >= 65.0f ||
       g_test_array[APP_SENSOR_O2].test_value >= 65.0f ||
       g_test_array[APP_SENSOR_H2S].test_value >= 65.0f ||
       g_test_array[APP_SENSOR_SO2].test_value >= 65.650f ||
       g_test_array[APP_SENSOR_NH3].test_value >= 65.0f)
    {
        alarm_flag = ALARM_FANG_DANGLE;
    }
    else if(g_test_array[APP_SENSOR_CO].test_value < 40.0f &&
            g_test_array[APP_SENSOR_CO2].test_value < 40.0f &&
            g_test_array[APP_SENSOR_O2].test_value < 40.0f &&
            g_test_array[APP_SENSOR_H2S].test_value < 40.0f &&
            g_test_array[APP_SENSOR_SO2].test_value < 40.650f &&
            g_test_array[APP_SENSOR_NH3].test_value < 40.0f)
    {
        alarm_flag = ALARM_FANG_SAFE;
    }
    else
    {
        alarm_flag = ALARM_FANG_WORN;
    }
}