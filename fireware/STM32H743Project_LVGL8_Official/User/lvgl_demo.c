
 
#include "lvgl_demo.h"
#include "./BSP/LED/led.h"
#include "FreeRTOS.h"
#include "task.h"

#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "LVGL/GUI_APP/lv_mainstart.h"
#include "app_runtime.h"


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

/* ALARM_TASK 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define ALARM_TASK_PRIO     5           /* 任务优先级 */
#define ALARM_STK_SIZE      128         /* 任务堆栈大小 */
TaskHandle_t AlarmTask_Handler;         /* 任务句柄 */
void alarm_task(void *pvParameters);    /* 任务函数 */

/* ANALYZE_TASK 任务 配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define ANALYZE_TASK_PRIO   4           /* 任务优先级 */
#define ANALYZE_STK_SIZE    128         /* 任务堆栈大小 */
TaskHandle_t AnalyzeTask_Handler;       /* 任务句柄 */
void analyze_task(void *pvParameters);  /* 任务函数 */

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

    /* 创建LVGL任务 */
    xTaskCreate((TaskFunction_t )lv_demo_task,
                (const char*    )"lv_demo_task",
                (uint16_t       )LV_DEMO_STK_SIZE, 
                (void*          )NULL,
                (UBaseType_t    )LV_DEMO_TASK_PRIO,
                (TaskHandle_t*  )&LV_DEMOTask_Handler);

    /* 告警任务 */
    xTaskCreate((TaskFunction_t )alarm_task,
                (const char*    )"alarm_task",
                (uint16_t       )ALARM_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )ALARM_TASK_PRIO,
                (TaskHandle_t*  )&AlarmTask_Handler);

    /* 数据分析任务 */
    xTaskCreate((TaskFunction_t )analyze_task,
                (const char*    )"analyze_task",
                (uint16_t       )ANALYZE_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )ANALYZE_TASK_PRIO,
                (TaskHandle_t*  )&AnalyzeTask_Handler);

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
        lv_timer_handler(); /* LVGL计时器 */
        vTaskDelay(5);
    }
}

/**
 * @brief       告警任务
 * @param       pvParameters : 传入参数(未用到)
 * @retval      无
 */
void alarm_task(void *pvParameters)
{
   while(1)
   {
       if(alarm_flag == ALARM_FANG_DANGLE)
       {
            LED1(1);
            LED0(0);
            app_interface_set_fan_enabled(true);
            app_interface_set_window_open(true);
        }
       else if(alarm_flag == ALARM_FANG_WORN)
       {
            LED0(1);
            LED1_TOGGLE();
            app_interface_set_fan_enabled(false);
            app_interface_set_window_open(true);
        }
       else if(alarm_flag == ALARM_FANG_SAFE)
       {    
            LED0(1);
            LED1(0);
            app_interface_set_fan_enabled(false);
            app_interface_set_window_open(false);
        }
       vTaskDelay(1000);
   }
}

void analyze_task(void *pvParameters)
{
    while(1)
    {
        app_runtime_analyze();
        vTaskDelay(1000);
    }
}

