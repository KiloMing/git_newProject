#ifndef APP_INTERFACE_H
#define APP_INTERFACE_H

/**
 * @file app_interface.h
 * @brief STM32业务数据与LVGL页面之间的统一接口。
 *
 * 本文件是应用层唯一建议对外包含的UI接口文件。移植到STM32后，传感器任务、
 * RTC任务以及执行器控制任务不需要了解page.c、setting.c、testvalue.c或chart.c
 * 内部创建了哪些LVGL对象，只需要调用本文件声明的函数。
 *
 * 推荐调用顺序：
 * 1. 初始化显示驱动和LVGL；
 * 2. 调用ui_init()创建全部页面，ui_init()内部最终会完成app_interface_init()；
 * 3. 在LVGL任务中调用app_interface_update_snapshot()刷新实时数据；
 * 4. 每小时整理24个历史数据点，并调用app_interface_set_history()；
 * 5. 电机或舵机任务通过app_interface_get_flag()读取用户开关状态。
 *
 * @warning LVGL不是线程安全的。所有会更新LVGL控件的set/update函数都应在
 *          LVGL任务中调用。如果传感器数据来自中断或其他RTOS任务，应先通过
 *          消息队列、邮箱或共享数据快照传给LVGL任务，不能在中断中直接调用。
 */

#include <stdbool.h>
#include <stdint.h>

#define APP_SENSOR_COUNT 6
#define APP_HISTORY_HOURS 24

/**
 * @brief 统一的气体编号。
 *
 * 该顺序必须与CurrentValues仪表盘、PastValues历史图和Settings状态灯保持一致。
 */
typedef enum {
    APP_SENSOR_CO = 0,
    APP_SENSOR_CO2,
    APP_SENSOR_O2,
    APP_SENSOR_H2S,
    APP_SENSOR_SO2,
    APP_SENSOR_NH3
} app_sensor_id_t;

/**
 * @brief 系统状态标志位。
 *
 * STM32可以通过app_interface_get_flags()读取风扇和舵机状态，并据此控制GPIO、
 * PWM或继电器。传感器检测任务可以通过app_interface_set_sensor_connected()
 * 写入连接状态，设置页LED会自动同步颜色。
 */
typedef enum {
    APP_FLAG_CO_CONNECTED   = (1UL << 0),
    APP_FLAG_CO2_CONNECTED  = (1UL << 1),
    APP_FLAG_O2_CONNECTED   = (1UL << 2),
    APP_FLAG_H2S_CONNECTED  = (1UL << 3),
    APP_FLAG_SO2_CONNECTED  = (1UL << 4),
    APP_FLAG_NH3_CONNECTED  = (1UL << 5),
    APP_FLAG_FAN_ENABLED    = (1UL << 6),
    APP_FLAG_WINDOW_OPEN    = (1UL << 7)
} app_state_flag_t;

/** @brief STM32 RTC传给界面使用的年月日、时分数据。 */
typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
} app_datetime_t;

/**
 * @brief 一次性刷新主界面所需的实时数据快照。
 *
 * gas_values数组顺序使用app_sensor_id_t。连接状态和执行器状态使用独立标志位，
 * 不放在该结构体中，避免实时数值刷新意外改变硬件控制状态。
 */
typedef struct {
    app_datetime_t datetime;
    float gas_values[APP_SENSOR_COUNT];
    float temperature_c;
    float humidity_percent;
} app_display_snapshot_t;

/**
 * @brief 当前系统状态标志。
 *
 * 建议通过下面的set/get函数访问。直接修改该变量不会自动刷新LVGL控件。
 */
extern volatile uint32_t g_app_state_flags;

/**
 * @brief 在所有页面创建完成后初始化统一接口并绑定两个LVGL开关事件。
 */
void app_interface_init(void);

/**
 * @brief 更新设置页的年月日和时分数字。
 * @param year   四位年份，例如2026。
 * @param month  月，建议范围1~12。
 * @param day    日，建议范围1~31。
 * @param hour   24小时制小时，建议范围0~23。
 * @param minute 分钟，建议范围0~59。
 */
void app_interface_set_datetime(uint16_t year,
                                uint8_t month,
                                uint8_t day,
                                uint8_t hour,
                                uint8_t minute);

/**
 * @brief 更新设置页的温度和湿度。
 * @param temperature_c 摄氏温度，例如26.5f。
 * @param humidity_percent 相对湿度百分比，例如58.0f。
 */
void app_interface_set_environment(float temperature_c, float humidity_percent);

/**
 * @brief 更新一个气体的实时测量值。
 *
 * 函数会自动刷新实际测量值、危险百分比和蓝/黄/红仪表盘颜色。
 */
void app_interface_set_gas_value(app_sensor_id_t sensor, float measured_value);

/**
 * @brief 一次性更新六种气体的实时值。
 * @param values 长度必须为APP_SENSOR_COUNT，顺序使用app_sensor_id_t。
 */
void app_interface_set_all_gas_values(const float values[APP_SENSOR_COUNT]);

/**
 * @brief 更新指定气体00:00~23:00的24小时历史数组。
 * @param sensor 要更新的气体编号。
 * @param values 长度必须为24；values[0]对应00:00，values[23]对应23:00。
 */
void app_interface_set_history(app_sensor_id_t sensor,
                               const float values[APP_HISTORY_HOURS]);

/**
 * @brief 设置传感器连接状态，并同步设置页LED。
 * @param sensor 需要更新的气体传感器编号。
 * @param connected true显示绿色LED，false显示红色LED。
 */
void app_interface_set_sensor_connected(app_sensor_id_t sensor, bool connected);

/**
 * @brief 设置或清除一个系统标志，并同步对应的LVGL控件。
 * @param flag 本次要操作的一个app_state_flag_t标志，不能同时传入多个标志。
 * @param enabled true置位，false清零。
 */
void app_interface_set_flag(app_state_flag_t flag, bool enabled);

/** @brief 查询某一个系统标志是否置位。 */
bool app_interface_get_flag(app_state_flag_t flag);

/** @brief 获取完整标志位，供STM32控制任务一次性读取。 */
uint32_t app_interface_get_flags(void);

/** @brief 程序主动设置风扇电机状态，并同步设置页开关。 */
void app_interface_set_fan_enabled(bool enabled);

/** @brief 程序主动设置舵机开窗状态，并同步设置页开关。 */
void app_interface_set_window_open(bool open);

/** @brief 使用一个结构体一次性刷新日期、时间、六种气体及温湿度。 */
void app_interface_update_snapshot(const app_display_snapshot_t *snapshot);

#endif /* APP_INTERFACE_H */
