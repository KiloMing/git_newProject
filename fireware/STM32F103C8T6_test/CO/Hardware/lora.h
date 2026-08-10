#ifndef __LORA_H
#define __LORA_H

#include "stm32f10x.h"

#define LORA_PACKET_MAGIC_1      0x4CU
#define LORA_PACKET_MAGIC_2      0x52U
#define LORA_PACKET_VERSION      0x01U
#define LORA_PACKET_OVERHEAD     8U
#define LORA_MAX_PAYLOAD         248U

/*
 * 与H743工程一致的定点传输协议：
 * 数组：[ADDR_H][ADDR_L][CHANNEL][4C][52][01][01][LEN_H][LEN_L][DATA...][CRC_H][CRC_L]
 * 文本：[ADDR_H][ADDR_L][CHANNEL][4C][52][01][02][LEN_H][LEN_L][TEXT...][CRC_H][CRC_L]
 * CRC-16/CCITT-FALSE校验范围：版本、类型、长度和有效数据。
 */

typedef enum
{
    LORA_PACKET_ARRAY = 0x01U,
    LORA_PACKET_TEXT  = 0x02U
} lora_packet_type_t;

typedef enum
{
    LORA_OK = 0,
    LORA_NO_DATA,
    LORA_INVALID_ARGUMENT,
    LORA_PAYLOAD_TOO_LONG,
    LORA_UART_ERROR,
    LORA_BAD_FRAME,
    LORA_CRC_ERROR,
    LORA_TYPE_MISMATCH,
    LORA_BUFFER_TOO_SMALL
} lora_status_t;

typedef struct
{
    lora_packet_type_t type;
    uint16_t length;
    uint8_t data[LORA_MAX_PAYLOAD + 1U];
} lora_packet_t;

/* USART1：PA9(TX)、PA10(RX)，8N1，LoRa模块使用定点传输模式。 */
lora_status_t lora_init(uint32_t baudrate);

/* 定点单字节协议：[ADDR_H][ADDR_L][CHANNEL][DATA]。 */
lora_status_t lora_send_byte(uint8_t data, uint8_t address_high,
                             uint8_t address_low, uint8_t channel);
lora_status_t lora_receive_byte(uint8_t *data);

/*
 * 非阻塞接收一个字节：有数据返回LORA_OK，无数据立即返回LORA_NO_DATA。
 * 接收格式为[DATA]，LoRa模块已经去除定点传输的地址和信道字节。
 */
lora_status_t lora_receive_byte_anytime(uint8_t *data);

/* 定点数组协议：[地址][信道][4C][52][01][01][长度][数据][CRC]。 */
lora_status_t lora_send_array(const uint8_t *data, uint16_t length,
                              uint8_t address_high, uint8_t address_low,
                              uint8_t channel);
lora_status_t lora_receive_array(uint8_t *data, uint16_t capacity, uint16_t *length);

/* 定点文本协议：[地址][信道][4C][52][01][02][长度][文本][CRC]。 */
lora_status_t lora_send_text(const char *text, uint8_t address_high,
                             uint8_t address_low, uint8_t channel);
lora_status_t lora_receive_text(char *text, uint16_t capacity, uint16_t *length);

/* 阻塞等待并接收数组包或文本包。 */
lora_status_t lora_receive_packet(lora_packet_t *packet);

#endif
