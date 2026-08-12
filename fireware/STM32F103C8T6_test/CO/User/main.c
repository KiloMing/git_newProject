#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "lora.h"
#include "LED.h"
#include "DHT11.h"
#include "MQ7.h"

#define TEST_DATA 1U
#define FAN_OFF 2U
#define FAN_ON 3U
#define WINDOWS_OFF 4U
#define WINDOWS_ON 5U
#define DHT11_TEMPERATURE 6U
#define DHT11_HUMIDITY 7U
#define MQ7_CO 8U

#define DHT11_STARTUP_DELAY_MS 1000U
#define DHT11_SAMPLE_INTERVAL_MS 2000U
#define MQ7_SAMPLE_INTERVAL_MS 200U
#define DISPLAY_INTERVAL_MS 500U

static void EncodeScaledValue(float value, uint8_t data[2])
{
    uint16_t scaled_value;

    if (value <= 0.0f)
    {
        scaled_value = 0U;
    }
    else if (value >= 655.35f)
    {
        scaled_value = 0xFFFFU;
    }
    else
    {
        scaled_value = (uint16_t)(value * 100.0f + 0.5f);
    }

    data[0] = (uint8_t)(scaled_value >> 8);
    data[1] = (uint8_t)scaled_value;
}

static void DisplayMeasuredValue(uint8_t row, const char *label, float value)
{
    uint32_t scaled_value;

    if (value < 0.0f)
    {
        value = 0.0f;
    }
    scaled_value = (uint32_t)(value * 10.0f + 0.5f);

    OLED_ShowString(row, 1U, (char *)label);
    OLED_ShowNum(row, 4U, scaled_value / 10U, 3U);
    OLED_ShowChar(row, 7U, '.');
    OLED_ShowNum(row, 8U, scaled_value % 10U, 1U);
}

int main(void)
{
    uint16_t num = 0U;
    uint8_t sent_flag = 0U;
    uint8_t payload[2];
    uint8_t dht11_started = 0U;
    uint8_t dht11_valid = 0U;
    uint32_t now;
    uint32_t last_dht11_ms;
    uint32_t last_mq7_ms;
    uint32_t last_display_ms;

    Delay_TickInit();
    OLED_Init();
    OLED_Clear();
    lora_init(9600U);
    LED_Init();
    LED1_OFF();
    LED2_OFF();
    DHT11_Init();
    MQ7_Init();

    now = Delay_GetTick();
    last_dht11_ms = now;
    last_mq7_ms = now - MQ7_SAMPLE_INTERVAL_MS;
    last_display_ms = now - DISPLAY_INTERVAL_MS;

    while (1)
    {
        now = Delay_GetTick();

        if (dht11_started == 0U)
        {
            if ((uint32_t)(now - last_dht11_ms) >= DHT11_STARTUP_DELAY_MS)
            {
                if (DHT11_Read() != 0U)
                {
                    dht11_valid = 1U;
                }
                last_dht11_ms = now;
                dht11_started = 1U;
            }
        }
        else if ((uint32_t)(now - last_dht11_ms) >= DHT11_SAMPLE_INTERVAL_MS)
        {
            if (DHT11_Read() != 0U)
            {
                dht11_valid = 1U;
            }
            last_dht11_ms = now;
        }

        if ((uint32_t)(now - last_mq7_ms) >= MQ7_SAMPLE_INTERVAL_MS)
        {
            (void)MQ7_Read();
            last_mq7_ms = now;
        }

        if ((uint32_t)(now - last_display_ms) >= DISPLAY_INTERVAL_MS)
        {
            if (dht11_valid != 0U)
            {
                DisplayMeasuredValue(1U, "T:", DHT11_Temperature);
                DisplayMeasuredValue(2U, "H:", DHT11_Humidity);
            }
            else
            {
                OLED_ShowString(1U, 1U, "T:---.-");
                OLED_ShowString(2U, 1U, "H:---.-");
            }
            DisplayMeasuredValue(3U, "CO:", MQ7_CO_PPM);
            last_display_ms = now;
        }

        if (lora_receive_byte_anytime(&sent_flag) == LORA_OK)
        {
            if (sent_flag == TEST_DATA)
            {
                payload[0] = (uint8_t)(num >> 8);
                payload[1] = (uint8_t)num;
                lora_send_array(payload, 2U, 0x00U, 0x05U, 0x17U);
                num = (num < 1000U) ? (uint16_t)(num + 1U) : 0U;
            }
            else if (sent_flag == FAN_OFF)
            {
                LED1_OFF();
            }
            else if (sent_flag == FAN_ON)
            {
                LED1_ON();
            }
            else if (sent_flag == WINDOWS_OFF)
            {
                LED2_OFF();
            }
            else if (sent_flag == WINDOWS_ON)
            {
                LED2_ON();
            }
            else if (sent_flag == DHT11_TEMPERATURE)
            {
                EncodeScaledValue(DHT11_Temperature, payload);
                lora_send_array(payload, 2U, 0x00U, 0x05U, 0x17U);
            }
            else if (sent_flag == DHT11_HUMIDITY)
            {
                EncodeScaledValue(DHT11_Humidity, payload);
                lora_send_array(payload, 2U, 0x00U, 0x05U, 0x17U);
            }
            else if (sent_flag == MQ7_CO)
            {
                EncodeScaledValue(MQ7_CO_PPM, payload);
                lora_send_array(payload, 2U, 0x00U, 0x05U, 0x17U);
            }

            sent_flag = 0U;
        }
    }
}
