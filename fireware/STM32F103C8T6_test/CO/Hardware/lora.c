#include "lora.h"
#include "UART.h"
#include <string.h>

/** @brief 计算与H743端相同的CRC-16/CCITT-FALSE校验值。 */
static uint16_t lora_crc16_ccitt(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFFU;
    uint16_t i;
    uint8_t bit;

    for (i = 0U; i < length; i++)
    {
        crc ^= (uint16_t)data[i] << 8;
        for (bit = 0U; bit < 8U; bit++)
        {
            if ((crc & 0x8000U) != 0U)
            {
                crc = (uint16_t)((crc << 1) ^ 0x1021U);
            }
            else
            {
                crc <<= 1;
            }
        }
    }

    return crc;
}

/** @brief 自动添加包头、长度和CRC，然后通过USART1发送。 */
static lora_status_t lora_send_packet(lora_packet_type_t type,
                                      const uint8_t *data,
                                      uint16_t length,
                                      uint8_t address_high,
                                      uint8_t address_low,
                                      uint8_t channel)
{
    uint8_t frame[LORA_MAX_PAYLOAD + LORA_PACKET_OVERHEAD + 3U];
    uint16_t crc;

    if ((data == 0) && (length != 0U))
    {
        return LORA_INVALID_ARGUMENT;
    }
    if (length > LORA_MAX_PAYLOAD)
    {
        return LORA_PAYLOAD_TOO_LONG;
    }

    frame[0] = address_high;
    frame[1] = address_low;
    frame[2] = channel;
    frame[3] = LORA_PACKET_MAGIC_1;
    frame[4] = LORA_PACKET_MAGIC_2;
    frame[5] = LORA_PACKET_VERSION;
    frame[6] = (uint8_t)type;
    frame[7] = (uint8_t)(length >> 8);
    frame[8] = (uint8_t)length;

    if (length != 0U)
    {
        memcpy(&frame[9], data, length);
    }

    crc = lora_crc16_ccitt(&frame[5], (uint16_t)(4U + length));
    frame[9U + length] = (uint8_t)(crc >> 8);
    frame[10U + length] = (uint8_t)crc;

    UART_SendArray(frame, (uint16_t)(length + LORA_PACKET_OVERHEAD + 3U));
    return LORA_OK;
}

lora_status_t lora_init(uint32_t baudrate)
{
    if (baudrate == 0U)
    {
        return LORA_INVALID_ARGUMENT;
    }

    UART_Init(baudrate);
    return LORA_OK;
}

lora_status_t lora_send_byte(uint8_t data, uint8_t address_high,
                             uint8_t address_low, uint8_t channel)
{
    UART_SendByte(address_high);
    UART_SendByte(address_low);
    UART_SendByte(channel);
    UART_SendByte(data);
    return LORA_OK;
}

lora_status_t lora_receive_byte(uint8_t *data)
{
    if (data == 0)
    {
        return LORA_INVALID_ARGUMENT;
    }

    *data = UART_ReceiveByte();
    return LORA_OK;
}

lora_status_t lora_receive_byte_anytime(uint8_t *data)
{
    if (data == 0)
    {
        return LORA_INVALID_ARGUMENT;
    }

    if (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
    {
        return LORA_NO_DATA;
    }

    *data = (uint8_t)USART_ReceiveData(USART1);
    return LORA_OK;
}

lora_status_t lora_send_array(const uint8_t *data, uint16_t length,
                              uint8_t address_high, uint8_t address_low,
                              uint8_t channel)
{
    return lora_send_packet(LORA_PACKET_ARRAY, data, length,
                            address_high, address_low, channel);
}

lora_status_t lora_send_text(const char *text, uint8_t address_high,
                             uint8_t address_low, uint8_t channel)
{
    size_t length;

    if (text == 0)
    {
        return LORA_INVALID_ARGUMENT;
    }

    length = strlen(text);
    if (length > LORA_MAX_PAYLOAD)
    {
        return LORA_PAYLOAD_TOO_LONG;
    }

    return lora_send_packet(LORA_PACKET_TEXT,
                            (const uint8_t *)text,
                            (uint16_t)length,
                            address_high,
                            address_low,
                            channel);
}

lora_status_t lora_receive_packet(lora_packet_t *packet)
{
    uint8_t header[4];
    uint16_t received_crc;
    uint16_t calculated_crc;
    uint16_t i;
    uint8_t byte;

    if (packet == 0)
    {
        return LORA_INVALID_ARGUMENT;
    }

    /* 搜索包头4C 52，自动丢弃包头前的干扰字节。 */
    while (1)
    {
        byte = UART_ReceiveByte();
        if (byte != LORA_PACKET_MAGIC_1)
        {
            continue;
        }
        if (UART_ReceiveByte() == LORA_PACKET_MAGIC_2)
        {
            break;
        }
    }

    header[0] = UART_ReceiveByte(); /* 版本 */
    header[1] = UART_ReceiveByte(); /* 类型 */
    header[2] = UART_ReceiveByte(); /* 长度高字节 */
    header[3] = UART_ReceiveByte(); /* 长度低字节 */

    if (header[0] != LORA_PACKET_VERSION)
    {
        return LORA_BAD_FRAME;
    }
    if ((header[1] != (uint8_t)LORA_PACKET_ARRAY) &&
        (header[1] != (uint8_t)LORA_PACKET_TEXT))
    {
        return LORA_BAD_FRAME;
    }

    packet->length = (uint16_t)(((uint16_t)header[2] << 8) | header[3]);
    if (packet->length > LORA_MAX_PAYLOAD)
    {
        return LORA_BAD_FRAME;
    }

    for (i = 0U; i < packet->length; i++)
    {
        packet->data[i] = UART_ReceiveByte();
    }
    received_crc = (uint16_t)((uint16_t)UART_ReceiveByte() << 8);
    received_crc |= UART_ReceiveByte();

    /* CRC输入必须是连续的版本、类型、长度和数据区域。 */
    {
        uint8_t crc_data[LORA_MAX_PAYLOAD + 4U];
        memcpy(crc_data, header, 4U);
        memcpy(&crc_data[4], packet->data, packet->length);
        calculated_crc = lora_crc16_ccitt(crc_data,
                                          (uint16_t)(4U + packet->length));
    }

    if (received_crc != calculated_crc)
    {
        return LORA_CRC_ERROR;
    }

    packet->type = (lora_packet_type_t)header[1];
    packet->data[packet->length] = '\0';
    return LORA_OK;
}

lora_status_t lora_receive_array(uint8_t *data, uint16_t capacity, uint16_t *length)
{
    lora_packet_t packet;
    lora_status_t status;

    if ((data == 0) || (length == 0))
    {
        return LORA_INVALID_ARGUMENT;
    }

    status = lora_receive_packet(&packet);
    if (status != LORA_OK)
    {
        return status;
    }
    if (packet.type != LORA_PACKET_ARRAY)
    {
        return LORA_TYPE_MISMATCH;
    }

    *length = packet.length;
    if (capacity < packet.length)
    {
        return LORA_BUFFER_TOO_SMALL;
    }

    memcpy(data, packet.data, packet.length);
    return LORA_OK;
}

lora_status_t lora_receive_text(char *text, uint16_t capacity, uint16_t *length)
{
    lora_packet_t packet;
    lora_status_t status;

    if ((text == 0) || (length == 0) || (capacity == 0U))
    {
        return LORA_INVALID_ARGUMENT;
    }

    status = lora_receive_packet(&packet);
    if (status != LORA_OK)
    {
        return status;
    }
    if (packet.type != LORA_PACKET_TEXT)
    {
        return LORA_TYPE_MISMATCH;
    }

    *length = packet.length;
    if (capacity <= packet.length)
    {
        return LORA_BUFFER_TOO_SMALL;
    }

    memcpy(text, packet.data, packet.length);
    text[packet.length] = '\0';
    return LORA_OK;
}
