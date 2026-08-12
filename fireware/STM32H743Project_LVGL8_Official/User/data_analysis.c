#include "data_analysis.h"
/*FreeRTOS配置*/

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


void data_analysis_creat(void)
{
    taskENTER_CRITICAL();           /* 进入临界区 */

    /* 创建告警任务 */
    xTaskCreate((TaskFunction_t )alarm_task,
                (const char*    )"alarm_task",
                (uint16_t       )ALARM_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )ALARM_TASK_PRIO,
                (TaskHandle_t*  )&AlarmTask_Handler);

    /* 创建数据分析任务 */
    xTaskCreate((TaskFunction_t )analyze_task,
                (const char*    )"analyze_task",
                (uint16_t       )ANALYZE_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )ANALYZE_TASK_PRIO,
                (TaskHandle_t*  )&AnalyzeTask_Handler);

    taskEXIT_CRITICAL();            /* 退出临界区 */
    
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
            // app_interface_set_fan_enabled(false);
            // lora_send_comm_byte(FAN_OFF, 0x00, 0x04, 0x16);
            // vTaskDelay(100U);

            app_interface_set_window_open(true);
        }
       else if(alarm_flag == ALARM_FANG_SAFE)
       {    
            LED0(1);
            LED1(0);
            // app_interface_set_fan_enabled(false);
            // lora_send_comm_byte(FAN_OFF, 0x00, 0x04, 0x16);
            // vTaskDelay(100U);

            // app_interface_set_window_open(false);
            // lora_send_comm_byte(SENSER_OFF, 0x00, 0x04, 0x16);

        }
       vTaskDelay(1000);
   }
}
/**
 * @brief       数据分析任务
 * @param       pvParameters : 传入参数(未用到)
 * @retval      无
 */
void analyze_task(void *pvParameters)
{
    while(1)
    {
        app_runtime_analyze();
        vTaskDelay(1000);
    }
}
