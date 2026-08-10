#ifndef __LORA_H
#define __LORA_H

#include "./SYSTEM/usart/usart.h"

/* 自定义数据包的固定参数。 */
#define LORA_PACKET_MAGIC_1          0x4CU
#define LORA_PACKET_MAGIC_2          0x52U
#define LORA_PACKET_VERSION          0x01U
#define LORA_PACKET_OVERHEAD         8U
#define LORA_MAX_PAYLOAD             (UART1_RX_DMA_BUFFER_SIZE - LORA_PACKET_OVERHEAD)
#define LORA_UART_TIMEOUT_MS         1000U

/*
 * LoRa模块使用定点传输模式，发送格式如下：
 * [目标地址高字节][目标地址低字节][目标信道]
 * [0x4C][0x52][版本][类型][长度高字节][长度低字节][数据...][CRC高字节][CRC低字节]
 * 长度和CRC均按大端序传输。
 * CRC-16/CCITT-FALSE校验范围为：版本、类型、长度和数据区。
 */

/* 数据包类型。 */
typedef enum
{
    LORA_PACKET_ARRAY = 0x01U,   /* 二进制数组 */
    LORA_PACKET_TEXT  = 0x02U    /* ASCII/UTF-8文本 */
} lora_packet_type_t;

/* LoRa接口返回状态。 */
typedef enum
{
    LORA_OK = 0,                 /* 操作成功 */
    LORA_NO_DATA,                /* 当前没有收到数据 */
    LORA_INVALID_ARGUMENT,       /* 参数无效 */
    LORA_PAYLOAD_TOO_LONG,       /* 数据长度超过限制 */
    LORA_UART_ERROR,             /* UART收发错误 */
    LORA_BAD_FRAME,              /* 数据包格式错误 */
    LORA_CRC_ERROR,              /* CRC校验失败 */
    LORA_TYPE_MISMATCH,          /* 数据包类型不匹配 */
    LORA_BUFFER_TOO_SMALL        /* 用户接收缓冲区太小 */
} lora_status_t;

/* 解析后的LoRa数据包。文本包的data末尾会自动添加'\0'。 */
typedef struct
{
    lora_packet_type_t type;                  /* 数据类型 */
    uint16_t length;                          /* 有效数据长度 */
    uint8_t data[LORA_MAX_PAYLOAD + 1U];      /* 有效数据 */
} lora_packet_t;

/**
 * @brief 初始化LoRa串口和DMA接收。
 * @protocol 本函数不发送应用数据；串口协议为UART 8N1，波特率由baudrate指定。
 * @note 使用USART1：PA9(TX)、PA10(RX)。LoRa模块必须工作在定点传输模式。
 */
lora_status_t lora_init(uint32_t baudrate);

/**
 * @brief 发送一个裸字节。
 * @protocol [ADDR_H][ADDR_L][CHANNEL][DATA]，总长度固定为4字节。
 * @example lora_send_byte(1, 0x00, 0x05, 0x17)发送：00 05 17 01。
 */
lora_status_t lora_send_byte(uint8_t data, uint8_t address_high,
                             uint8_t address_low, uint8_t channel);

/**
 * @brief 非阻塞接收一个裸字节。
 * @protocol [DATA]，DMA本次必须恰好收到1字节，否则返回LORA_BAD_FRAME。
 * @note 与lora_send_byte()配对使用，不能用于接收lora_send_array()的数据包。
 */
lora_status_t lora_receive_byte(uint8_t *data);

/**
 * @brief 发送二进制数组包。
 * @protocol [ADDR_H][ADDR_L][CHANNEL][4C][52][01][01][LEN_H][LEN_L][DATA...][CRC_H][CRC_L]
 * @note 类型字节01表示数组；长度为大端序；CRC覆盖[01][01][LEN_H][LEN_L][DATA...]。
 */
lora_status_t lora_send_array(const uint8_t *data, uint16_t length,
                              uint8_t address_high, uint8_t address_low,
                              uint8_t channel);

/**
 * @brief 发送文本数据包。
 * @protocol [ADDR_H][ADDR_L][CHANNEL][4C][52][01][02][LEN_H][LEN_L][TEXT...][CRC_H][CRC_L]
 * @note 类型字节02表示文本；不会发送字符串结尾的'\0'；CRC计算方法与数组包相同。
 */
lora_status_t lora_send_text(const char *text, uint8_t address_high,
                             uint8_t address_low, uint8_t channel);

/**
 * @brief 非阻塞接收数组包或文本包。
 * @protocol 接收[4C][52][版本][类型][长度][数据][CRC16]，自动识别类型01或02并校验CRC。
 * @note 与lora_send_array()/lora_send_text()配对使用，不能接收裸字节协议。
 */
lora_status_t lora_receive_packet(lora_packet_t *packet);

/**
 * @brief 非阻塞接收二进制数组包。
 * @protocol 只接受[4C][52][01][01][LEN_H][LEN_L][DATA...][CRC_H][CRC_L]。
 * @note 发送端必须调用lora_send_array()；接收模块会剥离地址和信道；length返回有效长度。
 */
lora_status_t lora_receive_array(uint8_t *data, uint16_t capacity, uint16_t *length);

/**
 * @brief 非阻塞接收文本包。
 * @protocol 只接受[4C][52][01][02][LEN_H][LEN_L][TEXT...][CRC_H][CRC_L]。
 * @note 发送端必须调用lora_send_text()；接收模块会剥离地址和信道；末尾自动添加'\0'。
 */
lora_status_t lora_receive_text(char *text, uint16_t capacity, uint16_t *length);

#endif
