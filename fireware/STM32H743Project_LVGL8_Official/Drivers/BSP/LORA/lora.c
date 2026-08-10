#include "./BSP/LORA/lora.h"
#include <string.h>

/**
 * @brief 计算CRC-16/CCITT-FALSE校验值。
 * @param data   待校验数据地址。
 * @param length 待校验数据长度。
 * @return CRC16校验值。
 */
static uint16_t lora_crc16_ccitt(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFFU;
    uint16_t i;
    uint8_t bit;

    for (i = 0; i < length; i++)
    {
        crc ^= (uint16_t)data[i] << 8;
        for (bit = 0; bit < 8U; bit++)
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

/**
 * @brief 组装并发送一个完整LoRa数据包。
 * @param type   数据包类型，数组或文本。
 * @param data   有效数据地址。
 * @param length 有效数据长度。
 * @return LoRa状态码。
 */
static lora_status_t lora_send_packet(lora_packet_type_t type,
                                      const uint8_t *data,
                                      uint16_t length,
                                      uint8_t address_high,
                                      uint8_t address_low,
                                      uint8_t channel)
{
    uint8_t frame[UART1_RX_DMA_BUFFER_SIZE + 3U];
    uint16_t crc;
    uint16_t frame_length;

    if ((data == NULL) && (length != 0U))
    {
        return LORA_INVALID_ARGUMENT;
    }
    if (length > LORA_MAX_PAYLOAD)
    {
        return LORA_PAYLOAD_TOO_LONG;
    }

    /* 写入包头、协议版本、数据类型和数据长度。 */
    /* 定点传输的前三字节：目标地址高、目标地址低、目标信道。 */
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

    /* CRC不包含前面的两个魔数。 */
    crc = lora_crc16_ccitt(&frame[5], (uint16_t)(4U + length));
    frame[9U + length] = (uint8_t)(crc >> 8);
    frame[10U + length] = (uint8_t)crc;
    frame_length = (uint16_t)(3U + LORA_PACKET_OVERHEAD + length);

    if (HAL_UART_Transmit(&g_uart1_handle,
                          frame,
                          frame_length,
                          LORA_UART_TIMEOUT_MS) != HAL_OK)
    {
        return LORA_UART_ERROR;
    }

    return LORA_OK;
}

/**
 * @brief 从USART1 DMA接收区取走一帧数据。
 * @note 复制期间短暂关闭中断，防止DMA回调同时修改接收数据。
 * @param data 用户缓冲区，容量至少为UART1_RX_DMA_BUFFER_SIZE。
 * @return 实际取得的字节数，返回0表示当前没有数据。
 */
static uint16_t lora_take_uart_frame(uint8_t *data)
{
    uint32_t primask;
    uint16_t length;

    primask = __get_PRIMASK();
    __disable_irq();

    if (g_uart1_rx_ready == 0U)
    {
        if (primask == 0U)
        {
            __enable_irq();
        }
        return 0U;
    }

    length = g_uart1_rx_length;
    if (length > UART1_RX_DMA_BUFFER_SIZE)
    {
        length = UART1_RX_DMA_BUFFER_SIZE;
    }
    memcpy(data, g_uart1_rx_frame, length);
    g_uart1_rx_length = 0U;
    g_uart1_rx_ready = 0U;

    if (primask == 0U)
    {
        __enable_irq();
    }

    return length;
}

/**
 * @brief 初始化LoRa使用的USART1，并启动空闲线DMA接收。
 * @param baudrate 串口波特率，必须与LoRa模块一致。
 * @protocol UART为8数据位、无校验、1停止位；LoRa模块使用定点传输模式。
 * @return LoRa状态码。
 */
lora_status_t lora_init(uint32_t baudrate)
{
    if (baudrate == 0U)
    {
        return LORA_INVALID_ARGUMENT;
    }

    usart_init(baudrate);
    g_uart1_rx_length = 0U;
    g_uart1_rx_ready = 0U;

    if (dma_uart_receice_data() != HAL_OK)
    {
        return LORA_UART_ERROR;
    }

    return LORA_OK;
}

/**
 * @brief 发送一个带协议封装的二进制数组。
 * @protocol [ADDR_H][ADDR_L][CHANNEL][4C][52][01][01][LEN_H][LEN_L][DATA...][CRC_H][CRC_L]。
 * @note LEN和CRC为大端序，类型01代表数组包。
 */
lora_status_t lora_send_array(const uint8_t *data, uint16_t length,
                              uint8_t address_high, uint8_t address_low,
                              uint8_t channel)
{
    return lora_send_packet(LORA_PACKET_ARRAY, data, length,
                            address_high, address_low, channel);
}

/**
 * @brief 发送一个裸字节，不添加包头或CRC。
 * @param data 要发送的8位数据。
 * @protocol [ADDR_H][ADDR_L][CHANNEL][DATA]。
 * @example lora_send_byte(1, 0x00, 0x05, 0x17)发送00 05 17 01。
 */
lora_status_t lora_send_byte(uint8_t data, uint8_t address_high,
                             uint8_t address_low, uint8_t channel)
{
    uint8_t frame[4];

    frame[0] = address_high;
    frame[1] = address_low;
    frame[2] = channel;
    frame[3] = data;

    if (HAL_UART_Transmit(&g_uart1_handle,
                          frame,
                          sizeof(frame),
                          LORA_UART_TIMEOUT_MS) != HAL_OK)
    {
        return LORA_UART_ERROR;
    }

    return LORA_OK;
}

/**
 * @brief 非阻塞接收一个裸字节。
 * @param data 保存接收结果的地址。
 * @protocol [DATA]，只接受1个裸字节。
 * @note DMA本次必须恰好收到1字节，否则返回LORA_BAD_FRAME；不能接收数组包或文本包。
 */
lora_status_t lora_receive_byte(uint8_t *data)
{
    uint8_t frame[UART1_RX_DMA_BUFFER_SIZE];
    uint16_t length;

    if (data == NULL)
    {
        return LORA_INVALID_ARGUMENT;
    }

    length = lora_take_uart_frame(frame);
    if (length == 0U)
    {
        return LORA_NO_DATA;
    }
    if (length != 1U)
    {
        return LORA_BAD_FRAME;
    }

    *data = frame[0];
    return LORA_OK;
}

/**
 * @brief 发送一个带协议封装的字符串，结尾的'\0'不会发送。
 * @param text 以'\0'结束的字符串。
 * @protocol [ADDR_H][ADDR_L][CHANNEL][4C][52][01][02][LEN_H][LEN_L][TEXT...][CRC_H][CRC_L]。
 * @note 类型02代表文本包，LEN不包含字符串结尾的'\0'。
 */
lora_status_t lora_send_text(const char *text, uint8_t address_high,
                             uint8_t address_low, uint8_t channel)
{
    size_t length;

    if (text == NULL)
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

/**
 * @brief 非阻塞接收并解析一个完整的数据包。
 * @param packet 保存类型、长度和有效数据的结构体。
 * @protocol [4C][52][版本][类型][LEN_H][LEN_L][DATA...][CRC_H][CRC_L]。
 * @note 支持类型01数组包和类型02文本包，不接受裸字节协议。
 * @return LORA_OK表示成功，LORA_NO_DATA表示当前尚无数据。
 */
lora_status_t lora_receive_packet(lora_packet_t *packet)
{
    uint8_t frame[UART1_RX_DMA_BUFFER_SIZE];
    uint16_t raw_length;
    uint16_t payload_length;
    uint16_t expected_length;
    uint16_t received_crc;
    uint16_t calculated_crc;
    uint16_t offset;

    if (packet == NULL)
    {
        return LORA_INVALID_ARGUMENT;
    }

    raw_length = lora_take_uart_frame(frame);
    if (raw_length == 0U)
    {
        return LORA_NO_DATA;
    }

    /* 跳过包头前可能存在的干扰字节，搜索“LR”魔数。 */
    for (offset = 0U; (uint16_t)(offset + LORA_PACKET_OVERHEAD) <= raw_length; offset++)
    {
        if ((frame[offset] == LORA_PACKET_MAGIC_1) &&
            (frame[offset + 1U] == LORA_PACKET_MAGIC_2))
        {
            break;
        }
    }

    if ((uint16_t)(offset + LORA_PACKET_OVERHEAD) > raw_length)
    {
        return LORA_BAD_FRAME;
    }
    if (frame[offset + 2U] != LORA_PACKET_VERSION)
    {
        return LORA_BAD_FRAME;
    }
    if ((frame[offset + 3U] != (uint8_t)LORA_PACKET_ARRAY) &&
        (frame[offset + 3U] != (uint8_t)LORA_PACKET_TEXT))
    {
        return LORA_BAD_FRAME;
    }

    payload_length = (uint16_t)(((uint16_t)frame[offset + 4U] << 8) |
                                frame[offset + 5U]);
    if (payload_length > LORA_MAX_PAYLOAD)
    {
        return LORA_BAD_FRAME;
    }

    expected_length = (uint16_t)(LORA_PACKET_OVERHEAD + payload_length);
    if ((uint16_t)(offset + expected_length) > raw_length)
    {
        return LORA_BAD_FRAME;
    }

    /* 比较接收CRC和本地重新计算的CRC。 */
    received_crc = (uint16_t)(((uint16_t)frame[offset + 6U + payload_length] << 8) |
                              frame[offset + 7U + payload_length]);
    calculated_crc = lora_crc16_ccitt(&frame[offset + 2U],
                                      (uint16_t)(4U + payload_length));
    if (received_crc != calculated_crc)
    {
        return LORA_CRC_ERROR;
    }

    packet->type = (lora_packet_type_t)frame[offset + 3U];
    packet->length = payload_length;
    if (payload_length != 0U)
    {
        memcpy(packet->data, &frame[offset + 6U], payload_length);
    }
    packet->data[payload_length] = '\0';

    return LORA_OK;
}

/**
 * @brief 接收一个二进制数组包。
 * @param data     用户接收缓冲区。
 * @param capacity 用户缓冲区容量。
 * @param length   返回实际数据长度。
 * @protocol 只接受[4C][52][01][01][LEN_H][LEN_L][DATA...][CRC_H][CRC_L]。
 * @note 必须与lora_send_array()配对，裸数组不能使用本函数接收。
 */
lora_status_t lora_receive_array(uint8_t *data, uint16_t capacity, uint16_t *length)
{
    lora_packet_t packet;
    lora_status_t status;

    if ((data == NULL) || (length == NULL))
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

/**
 * @brief 接收一个文本包，并在字符串末尾添加'\0'。
 * @param text     用户字符串缓冲区。
 * @param capacity 字符串缓冲区容量，必须比有效文本长度至少大1。
 * @param length   返回有效文本长度，不包含'\0'。
 * @protocol 只接受[4C][52][01][02][LEN_H][LEN_L][TEXT...][CRC_H][CRC_L]。
 * @note 必须与lora_send_text()配对，裸字符串不能使用本函数接收。
 */
lora_status_t lora_receive_text(char *text, uint16_t capacity, uint16_t *length)
{
    lora_packet_t packet;
    lora_status_t status;

    if ((text == NULL) || (length == NULL) || (capacity == 0U))
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
