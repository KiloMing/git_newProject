#include "lcd_function.h"


/**
 * @brief 初始化检测数据
 * @param value  ： 数据的地址
 */
void sensor_value_init(SENSOR_VALUE *value)
{
    *value = (SENSOR_VALUE){0};
}

/**
 * @brief 初始化LCD屏幕功能
 * @param direction : 确定显示方向 ， 1表示横向， 0表示纵向
 * @param color     : 清屏时的颜色
 */
void lcd_function_init(uint8_t direction, uint16_t color)
{
    lcd_init();                                         /* 初始化LCD */
    lcd_display_on();                                   /* 开启显示 */
    lcd_display_dir(direction);
    lcd_clear(color);
}

/**
 * @brief LCD显示相关气体信息
 * @param value : 测量数据
 */
void lcd_function_UI(SENSOR_VALUE value)
{
    char temp_buff[50];
    snprintf(temp_buff, sizeof(temp_buff), "CO VALUE: %.4f ppm", value.CO_value);
    lcd_show_string(50, 50, 480, 32, 32, temp_buff, BLUE);

    snprintf(temp_buff, sizeof(temp_buff), "CH4 VALUE: %.4f ppm", value.CH4_value);
    lcd_show_string(50, 100, 480, 32, 32, temp_buff, BLUE);
    
    snprintf(temp_buff, sizeof(temp_buff), "NH3 VALUE: %.4f ppm", value.NH3_value);
    lcd_show_string(50, 150, 480, 32, 32, temp_buff, BLUE);

    snprintf(temp_buff, sizeof(temp_buff), "PM2.5 VALUE: %.4f um/m^3", value.PM2_5_value);
    lcd_show_string(50, 200, 480, 32, 32, temp_buff, BLUE);

    snprintf(temp_buff, sizeof(temp_buff), "NO VALUE: %.4f ppm", value.NO_value);
    lcd_show_string(50, 250, 480, 32, 32, temp_buff, BLUE);

    snprintf(temp_buff, sizeof(temp_buff), "H2S VALUE: %.4f ppm", value.H2S_value);
    lcd_show_string(50, 300, 480, 32, 32, temp_buff, BLUE);
}
