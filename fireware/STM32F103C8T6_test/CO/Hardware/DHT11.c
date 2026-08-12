#include "DHT11.h"
#include "Delay.h"

#define DHT11_TIMEOUT_US 120U

float DHT11_Temperature = 0.0f;
float DHT11_Humidity = 0.0f;

static void DHT11_SetOutput(void)
{
    GPIO_InitTypeDef gpio_init;

    gpio_init.GPIO_Pin = DHT11_GPIO_PIN;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(DHT11_GPIO_PORT, &gpio_init);
}

static void DHT11_SetInput(void)
{
    GPIO_InitTypeDef gpio_init;

    gpio_init.GPIO_Pin = DHT11_GPIO_PIN;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(DHT11_GPIO_PORT, &gpio_init);
}

static uint8_t DHT11_WaitForLevel(BitAction level)
{
    uint16_t elapsed = 0U;

    while (GPIO_ReadInputDataBit(DHT11_GPIO_PORT, DHT11_GPIO_PIN) != level)
    {
        if (elapsed++ >= DHT11_TIMEOUT_US)
        {
            return 0U;
        }
        Delay_us(1U);
    }

    return 1U;
}

static uint8_t DHT11_ReadByte(uint8_t *value)
{
    uint8_t bit_index;
    uint8_t data = 0U;

    for (bit_index = 0U; bit_index < 8U; bit_index++)
    {
        if (DHT11_WaitForLevel(Bit_SET) == 0U)
        {
            return 0U;
        }

        Delay_us(30U);
        data <<= 1;
        if (GPIO_ReadInputDataBit(DHT11_GPIO_PORT, DHT11_GPIO_PIN) == Bit_SET)
        {
            data |= 1U;
            if (DHT11_WaitForLevel(Bit_RESET) == 0U)
            {
                return 0U;
            }
        }
    }

    *value = data;
    return 1U;
}

void DHT11_Init(void)
{
    RCC_APB2PeriphClockCmd(DHT11_GPIO_CLK, ENABLE);
    DHT11_SetOutput();
    GPIO_SetBits(DHT11_GPIO_PORT, DHT11_GPIO_PIN);
}

uint8_t DHT11_Read(void)
{
    uint8_t index;
    uint8_t data[5];
    float temperature;

    DHT11_SetOutput();
    GPIO_ResetBits(DHT11_GPIO_PORT, DHT11_GPIO_PIN);
    Delay_ms(20U);
    GPIO_SetBits(DHT11_GPIO_PORT, DHT11_GPIO_PIN);
    Delay_us(30U);
    DHT11_SetInput();

    if ((DHT11_WaitForLevel(Bit_RESET) == 0U) ||
        (DHT11_WaitForLevel(Bit_SET) == 0U) ||
        (DHT11_WaitForLevel(Bit_RESET) == 0U))
    {
        DHT11_SetOutput();
        GPIO_SetBits(DHT11_GPIO_PORT, DHT11_GPIO_PIN);
        return 0U;
    }

    for (index = 0U; index < 5U; index++)
    {
        if (DHT11_ReadByte(&data[index]) == 0U)
        {
            DHT11_SetOutput();
            GPIO_SetBits(DHT11_GPIO_PORT, DHT11_GPIO_PIN);
            return 0U;
        }
    }

    DHT11_SetOutput();
    GPIO_SetBits(DHT11_GPIO_PORT, DHT11_GPIO_PIN);

    if ((uint8_t)(data[0] + data[1] + data[2] + data[3]) != data[4])
    {
        return 0U;
    }

    DHT11_Humidity = (float)data[0] + ((float)data[1] / 10.0f);
    temperature = (float)(data[2] & 0x7FU) + ((float)data[3] / 10.0f);
    DHT11_Temperature = ((data[2] & 0x80U) != 0U) ? -temperature : temperature;

    return 1U;
}
