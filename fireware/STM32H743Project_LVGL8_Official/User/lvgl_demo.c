
 
#include "lvgl_demo.h"
#include "./BSP/LED/led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "./BSP/LORA/lora.h"
#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "LVGL/GUI_APP/lv_mainstart.h"
#include "app_runtime.h"
#include "lora_comm.h"
#include "data_analysis.h"


/******************************************************************************************************/
/*FreeRTOS配置*/

/* START_TASK 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define START_TASK_PRIO     1           /* 任务优先级 */
#define START_STK_SIZE      128         /* 任务堆栈大小 */
TaskHandle_t StartTask_Handler;         /* 任务句柄 */
void start_task(void *pvParameters);    /* 任务函数 */

/* LV_DEMO_TASK 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define LV_DEMO_TASK_PRIO   3           /* 任务优先级 */
#define LV_DEMO_STK_SIZE    1024        /* 任务堆栈大小 */
TaskHandle_t LV_DEMOTask_Handler;       /* 任务句柄 */
void lv_demo_task(void *pvParameters);  /* 任务函数 */

/******************************************************************************************************/


void lvgl_demo(void)
{
    lv_init();                                          /* lvgl系统初始化 */
    lv_port_disp_init();                                /* lvgl显示接口初始化,放在lv_init()的后面 */
    lv_port_indev_init();                               /* lvgl输入接口初始化,放在lv_init()的后面 */

    xTaskCreate((TaskFunction_t )start_task,            /* 任务函数 */
                (const char*    )"start_task",          /* 任务名称 */
                (uint16_t       )START_STK_SIZE,        /* 任务堆栈大小 */
                (void*          )NULL,                  /* 传递给任务函数的参数 */
                (UBaseType_t    )START_TASK_PRIO,       /* 任务优先级 */
                (TaskHandle_t*  )&StartTask_Handler);   /* 任务句柄 */

    vTaskStartScheduler();                              /* 开启任务调度 */
}

/**
 * @brief       start_task
 * @param       pvParameters : 传入参数(未用到)
 * @retval      无
 */
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           /* 进入临界区 */

    lora_task_creat();              /* 创建LoRa通信任务和发送队列 */
    data_analysis_creat();          /* 创建数据分析和告警任务 */

    /* 创建LVGL任务 */
    xTaskCreate((TaskFunction_t )lv_demo_task,
                (const char*    )"lv_demo_task",
                (uint16_t       )LV_DEMO_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )LV_DEMO_TASK_PRIO,
                (TaskHandle_t*  )&LV_DEMOTask_Handler);
    taskEXIT_CRITICAL();            /* 退出临界区 */
    vTaskDelete(StartTask_Handler); /* 删除开始任务 */
}

/**
 * @brief       LVGL运行例程
 * @param       pvParameters : 传入参数(未用到)
 * @retval      无
 */
void lv_demo_task(void *pvParameters)
{
    lv_mainstart();              /* 创建三个LVGL页面 */
    app_runtime_ui_start();      /* 启动RTC和模拟数据刷新 */
    
    while(1)
    {
        app_runtime_ui_process();
        fan_sensor_status_read();
        lv_timer_handler(); /* LVGL计时器 */
        vTaskDelay(5);
    }
}

/* 风扇传感器状态读取 */
void fan_sensor_status_read(void)
{
    static int8_t s_last_fan_status = -1;
    static int8_t s_last_sensor_status = -1;
    uint8_t fan_status;
    uint8_t sensor_status;

    fan_status = app_interface_get_flag(APP_FLAG_FAN_ENABLED) ? 1U : 0U;
    sensor_status = app_interface_get_flag(APP_FLAG_WINDOW_OPEN) ? 1U : 0U;

    if ((int8_t)fan_status != s_last_fan_status)
    {
        if (lora_send_comm_byte(fan_status != 0U ? FAN_ON : FAN_OFF,
                                0x00, 0x04, 0x16) == pdPASS)
        {
            s_last_fan_status = (int8_t)fan_status;
        }
    }

    if ((int8_t)sensor_status != s_last_sensor_status)
    {
        if (lora_send_comm_byte(sensor_status != 0U ? SENSER_ON : SENSER_OFF,
                                0x00, 0x04, 0x16) == pdPASS)
        {
            s_last_sensor_status = (int8_t)sensor_status;
        }
    }
}
