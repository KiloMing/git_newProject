#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/LORA/lora.h"

int main(void)
{
    uint8_t number_1[5];
    uint8_t number_2[5];
    sys_cache_enable();
    HAL_Init();
    sys_stm32_clock_init(240U, 2U, 2U, 4U);
    delay_init(480U);
    lora_init(9600U);

    lcd_init();
    lcd_display_dir(1U);
    lcd_clear(WHITE);
    lcd_show_string(20U, 30U, 300U, 32U, 24U, "LoRa receive:", BLUE);

    uint16_t len = 2;
    while (1)
    {
        lora_send_byte(1, 0x00, 0x04, 0x16);
        uint32_t start = HAL_GetTick();
        while (HAL_GetTick() - start < 1000U)
        {
            uint16_t len = sizeof(number_1);

            if (lora_receive_array(number_1, sizeof(number_1), &len) == LORA_OK)
            {
                /* 接收成功 */
                uint16_t t_num = (uint16_t)(number_1[0] << 8) | (number_1[1]);
                lcd_fill(20U, 80U, 220U, 120U, WHITE);
                lcd_show_num(20U, 80U, t_num, 3U, 32U, RED);
                break;
            }

            delay_ms(1U);
        }
        delay_ms(500);
        lora_send_byte(1, 0x00, 0x01, 0x13);
        uint32_t start_1 = HAL_GetTick();
        while (HAL_GetTick() - start_1 < 1000U)
        {
            uint16_t len = sizeof(number_2);

            if (lora_receive_array(number_2, sizeof(number_2), &len) == LORA_OK)
            {
                /* 接收成功 */
                uint16_t t_num = (uint16_t)(number_2[0] << 8) | (number_2[1]);
                lcd_fill(20U, 80U, 220U, 120U, WHITE);
                lcd_show_num(40U, 120U, t_num, 3U, 32U, RED);
                break;
            }

            delay_ms(1U);
        }  
            delay_ms(500);
    }       
    
}
