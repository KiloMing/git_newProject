#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "lora.h"
#include "LED.h"

int main(void)
{
    uint16_t num = 0;

    
    OLED_Init();
    OLED_Clear();
    lora_init(9600U);
    LED_Init();
    LED1_OFF();
    LED2_OFF();
    /* Allow the LoRa module to finish powering up. */
    Delay_ms(100);

    /* 自动发送：4C 52 01 01 00 02 01 02 CRC_H CRC_L。 */

    //主机地址 5，23, 主机进行询问，只有询问中才可以进行发送
    uint8_t sent_flag = 0;
    while (1)
    {
        uint8_t sent_set[2];
        sent_set[0] = (uint8_t)(num >> 8);
        sent_set[1] = (uint8_t)(num);
        if (lora_receive_byte_anytime(&sent_flag) == LORA_OK)
        {
            if (sent_flag == 1U)
            {
                OLED_ShowNum(1, 1, 99, 3);
                lora_send_array(sent_set, 2, 0x00, 0x05, 0x17);

                if (num < 1000U)
                {
                    num++;
                }
                else
                {
                    num = 0U;
                }
            }
            else if (sent_flag == 2U)
            {
                LED1_OFF();
            }
            else if (sent_flag == 3U)
            {
                LED1_ON();
            }
            else if (sent_flag == 4U)
            {
                LED2_OFF();
            }
            else if (sent_flag == 5U)
            {
                LED2_ON();
            }

            sent_flag = 0U;
        }
    }
}
