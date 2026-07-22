/**
 ****************************************************************************************************
 * @file        main.c
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2022-09-06
 * @brief       TFTLCD(MCU屏) 实验
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 阿波罗 H743开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/ADC/adc_1.h"
#include "./BSP/MPU/mpu.h"
#include "./BSP/LCD_FUNCTION/lcd_function.h"
#include "./BSP/RTC/rtc.h"

int main(void)
{

    sys_cache_enable();                                 /* 打开L1-Cache */
    HAL_Init();                                         /* 初始化HAL库 */
    sys_stm32_clock_init(160, 5, 2, 4);                 /* 设置时钟, 400Mhz */
    delay_init(400);                                    /* 延时初始化 */
    usart_init(115200);                                 /* 串口初始化 */
    //mpu_memory_protection();                            /* 保护相关存储区域 */

    lcd_function_init(1, WHITE);
    // SENSOR_VALUE test_value = {0};
    // sensor_value_init(&test_value);

    uint8_t hour, min, sec, ampm;
    uint8_t year, month, date, week;
    uint8_t tbuf[40];
    uint8_t t = 0;
    rtc_init();
    
    rtc_set_date(2026-48, 7, 22, 3);
    rtc_set_time(7,0,0);

    while (1)
    {
        // lcd_function_UI(test_value);
        // test_value.CO_value++;
        // test_value.H2S_value += 2.0f;
        // test_value.NH3_value += 5.0f;
        // delay_ms(200);

        t++;

        if ((t % 10) == 0)            /* 每100ms更新一次显示数据 */
        {
            rtc_get_time(&hour, &min, &sec, &ampm);
            sprintf((char *)tbuf, "Time:%02d:%02d:%02d", hour, min, sec);
            lcd_show_string(30, 130, 210, 16, 16, (char *)tbuf, RED);
            rtc_get_date(&year, &month, &date, &week);
            sprintf((char *)tbuf, "Date:20%02d-%02d-%02d", year, month, date);
            lcd_show_string(30, 150, 210, 16, 16, (char *)tbuf, RED);
            sprintf((char *)tbuf, "Week:%d", week);
            lcd_show_string(30, 170, 210, 16, 16, (char *)tbuf, RED);
        }
    }
}

