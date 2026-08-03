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
#include "./BSP/MPU/mpu.h"
#include "./BSP/LCD_FUNCTION/lcd_function.h"
#include "./BSP/RTC/rtc.h"
#include "./BSP/SDRAM/sdram.h"

#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "ui.h"
#include "app_interface.h"
#include "lv_arc_demo.h"

/*
 * 1：运行正点原子官方 lv_arc 的 LVGL9 适配页，单独验证底层移植。
 * 0：运行项目原有的三个标签页界面。
 */
#define APP_USE_ALIENTEK_ARC_DEMO 1

/**
 * @brief 为LVGL提供毫秒时基。
 *
 * HAL_GetTick()由SysTick中断每1ms自动更新，因此不需要在中断函数中额外调用
 * lv_tick_inc()，也避免了LVGL与HAL重复维护两套系统节拍。
 */
static uint32_t app_lvgl_get_tick(void)
{
    return HAL_GetTick();
}

int main(void)
{

    /*
     * 先配置MPU再开启Cache：LCD FMC区必须禁用Cache，外部SDRAM则设为可缓存普通内存。
     * 顺序反过来可能导致LCD写入滞留在D-Cache，表现为白屏或局部画面不刷新。
     */
    mpu_memory_protection();                            /* 先配置LCD/SDRAM属性，避免Cache导致异常刷新 */
    sys_cache_enable();                                 /* 打开L1-Cache */
    HAL_Init();                                         /* 初始化HAL库 */
    sys_stm32_clock_init(160, 5, 2, 4);                 /* 设置时钟, 400Mhz */
    delay_init(400);                                    /* 延时初始化 */
    usart_init(115200);                                 /* 串口初始化 */

    /* 外部SDRAM用于LVGL 1MB内存池和32KB局部绘图缓冲。 */
    sdram_init();

    /* 初始化FMC LCD并切换为横屏；lcddev中会保存实际识别到的分辨率。 */
    /* 与官方 LVGL 例程一致：LCD初始化后明确设置横屏。 */
    lcd_init();
    lcd_display_dir(1);
    lcd_clear(WHITE);
    printf("LCD ID: 0x%04X, resolution: %u x %u\r\n",
           (unsigned int)lcddev.id,
           (unsigned int)lcddev.width,
           (unsigned int)lcddev.height);

    /* LVGL初始化顺序：核心 -> HAL毫秒时基 -> 显示移植层 -> 应用页面。 */
    lv_init();
    lv_tick_set_cb(app_lvgl_get_tick);
    lv_port_disp_init();
    lv_port_indev_init();
#if APP_USE_ALIENTEK_ARC_DEMO
    lv_arc_demo_create();
#else
    ui_init();
#endif
    // SENSOR_VALUE test_value = {0};
    // sensor_value_init(&test_value);

    // uint8_t hour, min, sec, ampm;
    // uint8_t year, month, date, week;
    // uint8_t tbuf[40];
    // uint8_t t = 0;
    // rtc_init();
    
    // rtc_set_date(2026-48, 7, 22, 3);
    // rtc_set_time(7,0,0);
    // printf("wew\r\n");
    // dma_uart_receice_data();

    while (1)
    {
        /*
         * 第一阶段只验证LCD和LVGL界面显示，不启动ADC/DMA，也不更新模拟数据。
         * page.c创建控件时设置的初始测量值会保持不变。
         */
        (void)lv_timer_handler();
        HAL_Delay(5U);

    }
}

