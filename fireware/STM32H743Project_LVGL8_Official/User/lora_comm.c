#include "lora_comm.h"

volatile uint16_t co_value_test = 0;
volatile uint16_t co2_value_test = 0;
volatile uint16_t temperature_value_test = 0;
volatile uint16_t humidity_value_test = 0;
#define TEST_DATA 1
#define FAN_OFF 2
#define FAN_ON 3
#define WINDOWS_OFF 4
#define WINDOWS_ON 5
#define DHT11_TEMPERATURE 6
#define DHT11_HUMIDITY 7
#define CO 8
/****************************/
/**
 * @brief       LoRa发送数据结构体
 * @param       data : 发送的数据   
 * @param       add_high : 发送数据的高地址
 * @param       add_low : 发送数据的低地址
 * @param       channel : 发送数据的通道
 * @retval      无
 */
typedef struct
{
    uint8_t data;
    uint8_t add_high;
    uint8_t add_low;
    uint8_t channel;
} lora_tx_request_t;
/****************************/

/***************************/

/* lora发送队列 */
static QueueHandle_t lora_tx_queue_handler = NULL;

/* lora发送句柄 */
static TaskHandle_t lora_tx_task_handler = NULL;

/* 传感器轮询接收句柄 */
static TaskHandle_t sensor_polling_task_handler = NULL;

/***************************/
/**
 * LORA_TX_TASK 任务配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define LORA_TX_TASK_PRIO   4      /* 任务优先级 */
#define LORA_TX_STK_SIZE    256    /* LoRa协议解析需要较大的局部缓冲区 */
static void lora_tx_task(void *pvParameters); /* 任务函数 */


/**
 * RECEIVE_DATA_TASK 任务配置
 * 包括: 任务句柄 任务优先级 堆栈大小 创建任务
 */
#define SENSOR_POLLING_TASK_PRIO   4      /* 任务优先级 */
#define SENSOR_POLLING_STK_SIZE    256    /* LoRa协议解析需要较大的局部缓冲区 */
static void sensor_polling_task(void *pvParameters); /* 任务函数 */


void lora_task_creat(void)
{
    /* 创建LoRa发送队列 */
    lora_tx_queue_handler = xQueueCreate(LORA_TX_QUEUE_LENGTH, sizeof(lora_tx_request_t));

    /* 创建失败 */
    if (lora_tx_queue_handler == NULL)
    {
        return;
    }
    /* lora_tx 任务 */
    xTaskCreate((TaskFunction_t )lora_tx_task,
                (const char*    )"lora_tx_task",
                (uint16_t       )LORA_TX_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )LORA_TX_TASK_PRIO,
                (TaskHandle_t*  )&lora_tx_task_handler);

    /* 传感器轮询任务 */
    xTaskCreate((TaskFunction_t )sensor_polling_task,
                (const char*    )"sensor_polling_task",
                (uint16_t       )SENSOR_POLLING_STK_SIZE,
                (void*          )NULL,
                (UBaseType_t    )SENSOR_POLLING_TASK_PRIO,
                (TaskHandle_t*  )&sensor_polling_task_handler);
}


BaseType_t lora_send_comm_byte(uint8_t data, uint8_t add_high, uint8_t add_low, uint8_t channel)
{
    lora_tx_request_t tx_request;
    /* Queue 未创建 */
    if (lora_tx_queue_handler == NULL)
    {
        return pdFAIL; // 队列未创建，发送失败
    }
    tx_request.data = data;
    tx_request.add_high = add_high;
    tx_request.add_low = add_low;
    tx_request.channel = channel;

    /* 将整个tx_request复制到Queue；每个调用只入队一次。 */
    return xQueueSendToBack(lora_tx_queue_handler,
                            &tx_request,
                            pdMS_TO_TICKS(20));
}

/**
 * @brief       LoRa发送任务
 * @param       pvParameters : 传入参数(未用到)
 * @retval      无
 */
static void lora_tx_task(void *pvParameters)
{
    lora_tx_request_t tx_request;
    (void)pvParameters; // 避免未使用参数的警告
    while (1)
    {
        /* Queue为空时，不会阻塞 */
        /* 从队列中接收发送请求 */
        if (xQueueReceive(lora_tx_queue_handler, &tx_request, portMAX_DELAY) == pdPASS)
        {
            /* 发送数据 */
            lora_send_byte(tx_request.data, tx_request.add_high, tx_request.add_low, tx_request.channel);
        }
    }
}

/**
 * @brief       传感器轮询接收任务
 * @param       pvParameters : 传入参数(未用到)
 * @retval      无
 */
static void sensor_polling_task(void *pvParameters)
{
    (void)pvParameters; // 避免未使用参数的警告
    uint8_t num_set_0[5];
    uint8_t num_set_1[5];

    uint32_t start_co;
    uint32_t start_co2;
    uint32_t temp_start;
    uint32_t hum_start;

    while (1)
    {   
        // co 4 22
        lora_send_comm_byte(CO, 0x00, 0x04, 0x16);
        start_co = HAL_GetTick();
        while (HAL_GetTick() - start_co < 1000U)
        {
            uint16_t len = sizeof(num_set_0);
            if (lora_receive_array(num_set_0, sizeof(num_set_0), &len) == LORA_OK)
            {
                if (len == 2U)
                {
                    co_value_test = ((uint16_t)num_set_0[0] << 8) | num_set_0[1];
                    app_runtime_record_co_sample((float)co_value_test / 100.0f);
                }
                app_interface_set_sensor_connected(APP_SENSOR_CO, true);
                break;
            }
            else
            {
                app_interface_set_sensor_connected(APP_SENSOR_CO, false);
            }
            vTaskDelay(1U);
        }
        vTaskDelay(500U);

        // co2 1 19
        lora_send_comm_byte(TEST_DATA, 0x00, 0x01, 0x13);
        start_co2 = HAL_GetTick();
        while (HAL_GetTick() - start_co2 < 1000U)
        {
            uint16_t len = sizeof(num_set_1);
            if (lora_receive_array(num_set_1, sizeof(num_set_1), &len) == LORA_OK)
            {
                if (len == 2U)
                {
                    co2_value_test = ((uint16_t)num_set_1[0] << 8) | num_set_1[1];
                }
                app_interface_set_sensor_connected(APP_SENSOR_CO2, true);
                break;
            }
            else
            {
                app_interface_set_sensor_connected(APP_SENSOR_CO2, false);
            }
            vTaskDelay(1U);
        }
        vTaskDelay(500U);
        // temperature 1 19
        lora_send_comm_byte(DHT11_TEMPERATURE, 0x00, 0x01, 0x13);
        temp_start = HAL_GetTick();
        while (HAL_GetTick() - temp_start < 1000U)
        {
            uint16_t len = sizeof(num_set_0);
            if (lora_receive_array(num_set_0, sizeof(num_set_0), &len) == LORA_OK)
            {
                if (len == 2U)
                {
                    temperature_value_test = ((uint16_t)num_set_0[0] << 8) | num_set_0[1];
                }
                break;
            }
            vTaskDelay(1U);
        }
        vTaskDelay(500U);
        // humidity 1 19
        lora_send_comm_byte(DHT11_HUMIDITY, 0x00, 0x01, 0x13);
        hum_start = HAL_GetTick();
        while (HAL_GetTick() - hum_start < 1000U)
        {
            uint16_t len = sizeof(num_set_1);
            if (lora_receive_array(num_set_1, sizeof(num_set_1), &len) == LORA_OK)
            {
                if (len == 2U)
                {
                    humidity_value_test = ((uint16_t)num_set_1[0] << 8) | num_set_1[1];
                }
                
                break;
            }

            vTaskDelay(1U);
        }
        vTaskDelay(500U);
    }
}
