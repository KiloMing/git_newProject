/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @date        2023-04-01
 * @brief       LVGL lv_arc(圆弧) 实验
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:STM32H743开发板
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/SDRAM/sdram.h"
#include "./BSP/LED/led.h"
#include "./BSP/KEY/key.h"
#include "./BSP/MPU/mpu.h"
#include "./BSP/TOUCH/touch.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/LORA/lora.h"
#include "lvgl_demo.h"
#include "app_runtime.h"


int main(void)
{
    sys_cache_enable();                                         /* 打开L1-Cache */
    HAL_Init();                                                 /* 初始化HAL库 */
    sys_stm32_clock_init(192, 5, 2, 4);                         /* 设置时钟, 480Mhz */
    delay_init(480);                                            /* 延时初始化 */
    lora_init(9600U);                                           /* LoRa串口和DMA接收初始化 */
    mpu_memory_protection();                                    /* 保护相关存储区域 */
    led_init();                                                 /* 初始化LED */
    key_init();                                                 /* 初始化KEY */
    sdram_init();                                               /* 初始化SDRAM */
    app_runtime_hardware_init();                                /* 初始化RTC */

    /* 电阻屏坐标矫正 */
    if (key_scan(0) == KEY0_PRES)                               /* KEY0按下,则执行校准程序 */
    {
        lcd_clear(WHITE);                                       /* 清屏 */
        tp_adjust();                                            /* 屏幕校准 */
        tp_save_adjust_data();
    }
    
    lvgl_demo();                                                /* 运行FreeRTOS例程 */
    return 0;
}
