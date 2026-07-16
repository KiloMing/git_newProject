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


int main(void)
{

    //sys_cache_enable();                                 /* 打开L1-Cache */
    HAL_Init();                                         /* 初始化HAL库 */
    sys_stm32_clock_init(160, 5, 2, 4);                 /* 设置时钟, 400Mhz */
    delay_init(400);                                    /* 延时初始化 */
    usart_init(115200);                                 /* 串口初始化 */
   // mpu_memory_protection();                            /* 保护相关存储区域 */
    led_init();                                         /* 初始化LED */
    lcd_init();                                         /* 初始化LCD */
    //uint16_t value = 0;
    adc_1_init();
    lcd_clear(WHITE);
    adc_dma_1_start();
    while (1)
    {
        
        adc_1_lcd_voltage_dma();
        LED0_TOGGLE();     /* 红灯闪烁 */
        delay_ms(50);
    }
}
