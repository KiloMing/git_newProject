/**
 * @file        lora_comm.h
 * @author      KiloMing
 * @version     V1.0
 * @date        2024-06-10
 * @brief       LoRa通信任务头文件,本文件不做底层接收，只做对应的接收发送信息的处理
 */

#ifndef __LORA_COMM_H
#define __LORA_COMM_H

#include "app_runtime.h"
#include "lvgl_demo.h"
#include "./BSP/LED/led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "./BSP/LORA/lora.h"
#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "LVGL/GUI_APP/lv_mainstart.h"
#include "app_runtime.h"

#define LORA_TX_QUEUE_LENGTH 10   /* LoRa发送队列长度 */

extern volatile uint16_t co_value_test;
extern volatile uint16_t co2_value_test;
extern volatile uint16_t temperature_value_test;
extern volatile uint16_t humidity_value_test;

BaseType_t lora_send_comm_byte(uint8_t data, uint8_t add_high, uint8_t add_low, uint8_t channel);   /* LoRa发送数据函数 */
void lora_task_creat(void);   /* LoRa通信任务创建函数 */

#endif 
