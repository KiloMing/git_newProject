#ifndef __LCD_FUNCTION_
#define __LCD_FUNCTION_

#include "stdio.h"
#include "./SYSTEM/sys/sys.h"
#include "./BSP/LCD/lcd.h"

typedef struct{
    float CO_value;
    float CH4_value;
    float PM2_5_value;
    float NH3_value;
    float NO_value;
    float H2S_value;
}SENSOR_VALUE;

void sensor_value_init(SENSOR_VALUE *value);
void lcd_function_init(uint8_t direction, uint16_t color);
void lcd_function_UI(SENSOR_VALUE value);














#endif
