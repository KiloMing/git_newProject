#include "UART.h"

static const char UART_HexTable[] = "0123456789ABCDEF";

void UART_Init(uint32_t BaudRate)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                           RCC_APB2Periph_USART1,
                           ENABLE);

    /* USART1_TX: PA9, alternate-function push-pull output. */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART1_RX: PA10, floating input. */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = BaudRate;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART1, &USART_InitStructure);

    USART_Cmd(USART1, ENABLE);
}

void UART_SendByte(uint8_t Byte)
{
    USART_SendData(USART1, Byte);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
    {
    }
}

void UART_SendArray(const uint8_t *Data, uint16_t Length)
{
    uint16_t i;

    if (Data == 0)
    {
        return;
    }

    for (i = 0; i < Length; i++)
    {
        UART_SendByte(Data[i]);
    }
}

void UART_SendString(const char *String)
{
    if (String == 0)
    {
        return;
    }

    while (*String != '\0')
    {
        UART_SendByte((uint8_t)*String);
        String++;
    }
}

void UART_SendHexByte(uint8_t Byte)
{
    UART_SendByte((uint8_t)UART_HexTable[Byte >> 4]);
    UART_SendByte((uint8_t)UART_HexTable[Byte & 0x0F]);
}

void UART_SendHex(uint32_t Number, uint8_t Length)
{
    uint8_t i;
    uint8_t Shift;

    if (Length > 8)
    {
        Length = 8;
    }

    for (i = 0; i < Length; i++)
    {
        Shift = (uint8_t)((Length - i - 1) * 4);
        UART_SendByte((uint8_t)UART_HexTable[(Number >> Shift) & 0x0F]);
    }
}

void UART_SendHexArray(const uint8_t *Data, uint16_t Length, char Separator)
{
    uint16_t i;

    if (Data == 0)
    {
        return;
    }

    for (i = 0; i < Length; i++)
    {
        UART_SendHexByte(Data[i]);
        if ((Separator != '\0') && (i + 1 < Length))
        {
            UART_SendByte((uint8_t)Separator);
        }
    }
}

uint8_t UART_ReceiveByte(void)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
    {
    }

    return (uint8_t)USART_ReceiveData(USART1);
}

uint16_t UART_ReceiveString(char *Buffer, uint16_t BufferSize)
{
    uint8_t Byte;
    uint16_t Length = 0;

    if ((Buffer == 0) || (BufferSize == 0))
    {
        return 0;
    }

    while (1)
    {
        Byte = UART_ReceiveByte();

        /* Ignore an empty CR/LF, including the LF left by a CR-LF pair. */
        if ((Byte == '\r') || (Byte == '\n'))
        {
            if (Length == 0)
            {
                continue;
            }
            break;
        }

        /* Support backspace and Delete from a serial terminal. */
        if ((Byte == '\b') || (Byte == 0x7F))
        {
            if (Length > 0)
            {
                Length--;
            }
            continue;
        }

        if (Length < (uint16_t)(BufferSize - 1))
        {
            Buffer[Length] = (char)Byte;
            Length++;
        }
    }

    Buffer[Length] = '\0';
    return Length;
}

uint8_t UART_ParseInt32(const char *String, int32_t *Number)
{
    uint32_t Value = 0;
    uint32_t Limit;
    uint8_t Negative = 0;
    uint8_t DigitFound = 0;
    uint8_t Digit;

    if ((String == 0) || (Number == 0))
    {
        return 0;
    }

    while ((*String == ' ') || (*String == '\t'))
    {
        String++;
    }

    if ((*String == '+') || (*String == '-'))
    {
        Negative = (*String == '-') ? 1 : 0;
        String++;
    }

    Limit = Negative ? 2147483648UL : 2147483647UL;

    while ((*String >= '0') && (*String <= '9'))
    {
        DigitFound = 1;
        Digit = (uint8_t)(*String - '0');
        if (Value > (Limit - Digit) / 10UL)
        {
            return 0;
        }
        Value = Value * 10UL + Digit;
        String++;
    }

    while ((*String == ' ') || (*String == '\t'))
    {
        String++;
    }

    if ((DigitFound == 0) || (*String != '\0'))
    {
        return 0;
    }

    if (Negative != 0)
    {
        if (Value == 2147483648UL)
        {
            *Number = (int32_t)0x80000000UL;
        }
        else
        {
            *Number = -(int32_t)Value;
        }
    }
    else
    {
        *Number = (int32_t)Value;
    }

    return 1;
}

uint8_t UART_ReceiveInt32(int32_t *Number)
{
    char Buffer[24];

    UART_ReceiveString(Buffer, sizeof(Buffer));
    return UART_ParseInt32(Buffer, Number);
}

void UART_SendLora_num(const uint8_t *Data, uint16_t Length,
                       uint8_t AddressHigh, uint8_t AddressLow, uint8_t Channel)
{
    UART_SendByte(AddressHigh);
    UART_SendByte(AddressLow);
    UART_SendByte(Channel);
    UART_SendArray(Data, Length);
}
