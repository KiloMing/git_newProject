#include "stm32f10x.h"
#include "OLED.h"
#include "UART.h"

/*
 * LoRa receiver example.
 * This file is not added to the Keil project because main.c is the transmitter.
 * To build the receiver firmware, exclude main.c and add this file instead.
 */
int main(void)
{
    uint8_t Number;

    OLED_Init();
    OLED_Clear();
    UART_Init(9600);

    OLED_ShowString(1, 1, "Lora Wait...");

    while (1)
    {
        /* The receiving LoRa module normally outputs only the payload byte. */
        Number = UART_ReceiveByte();

        OLED_Clear();
        OLED_ShowString(1, 1, "Lora Receive:");
        OLED_ShowNum(2, 1, Number, 3);
    }
}
