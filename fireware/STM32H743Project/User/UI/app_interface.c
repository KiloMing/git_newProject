#include "app_interface.h"

#include "chart.h"
#include "page.h"
#include "setting.h"
#include "testvalue.h"

/**
 * @brief 全局系统标志位。
 *
 * 初始值为0：六路传感器均显示未连接，风扇关闭，窗户关闭。
 * volatile用于提醒编译器该值可能被其他执行上下文读取，但volatile本身不提供
 * 多线程原子性；在RTOS中若多个任务同时写入，仍应使用临界区或互斥锁。
 */
volatile uint32_t g_app_state_flags = 0U;

/** 气体编号到传感器连接标志位的固定映射。 */
static const app_state_flag_t sensor_connection_flags[APP_SENSOR_COUNT] = {
    APP_FLAG_CO_CONNECTED,
    APP_FLAG_CO2_CONNECTED,
    APP_FLAG_O2_CONNECTED,
    APP_FLAG_H2S_CONNECTED,
    APP_FLAG_SO2_CONNECTED,
    APP_FLAG_NH3_CONNECTED
};

/** @brief 检查气体编号，防止数组越界。 */
static bool app_sensor_id_valid(app_sensor_id_t sensor)
{
    return sensor >= APP_SENSOR_CO && sensor < APP_SENSOR_COUNT;
}

/** @brief 根据标志位刷新某一路传感器LED颜色。 */
static void app_interface_sync_sensor_led(app_sensor_id_t sensor)
{
    if(!app_sensor_id_valid(sensor) || g_setting_ui.sensor_led[sensor] == NULL) {
        return;
    }

    bool connected = app_interface_get_flag(sensor_connection_flags[sensor]);
    lv_color_t color = connected
                       ? lv_palette_main(LV_PALETTE_GREEN)
                       : lv_palette_main(LV_PALETTE_RED);
    lv_led_set_color(g_setting_ui.sensor_led[sensor], color);
    lv_led_set_brightness(g_setting_ui.sensor_led[sensor], 255);
}

/** @brief 根据风扇和舵机标志同步开关选中状态与文字。 */
static void app_interface_sync_control_widgets(void)
{
    bool fan_enabled = app_interface_get_flag(APP_FLAG_FAN_ENABLED);
    bool window_open = app_interface_get_flag(APP_FLAG_WINDOW_OPEN);

    if(g_setting_ui.fan_switch != NULL) {
        if(fan_enabled) {
            lv_obj_add_state(g_setting_ui.fan_switch, LV_STATE_CHECKED);
        }
        else {
            lv_obj_remove_state(g_setting_ui.fan_switch, LV_STATE_CHECKED);
        }
    }
    if(g_setting_ui.fan_state_label != NULL) {
        lv_label_set_text(g_setting_ui.fan_state_label, fan_enabled ? "ON" : "OFF");
    }

    if(g_setting_ui.window_switch != NULL) {
        if(window_open) {
            lv_obj_add_state(g_setting_ui.window_switch, LV_STATE_CHECKED);
        }
        else {
            lv_obj_remove_state(g_setting_ui.window_switch, LV_STATE_CHECKED);
        }
    }
    if(g_setting_ui.window_state_label != NULL) {
        lv_label_set_text(g_setting_ui.window_state_label, window_open ? "OPEN" : "CLOSED");
    }
}

/**
 * @brief 两个LVGL开关共用的事件回调。
 *
 * user_data保存对应的标志位。用户点击开关时只改变标志位，不直接操作GPIO。
 * STM32电机/舵机任务读取标志位后再执行真正的硬件控制，实现UI与驱动解耦。
 */
static void app_interface_switch_event_cb(lv_event_t *event)
{
    lv_obj_t *switch_obj = lv_event_get_target_obj(event);
    app_state_flag_t flag = (app_state_flag_t)(uintptr_t)lv_event_get_user_data(event);
    bool enabled = lv_obj_has_state(switch_obj, LV_STATE_CHECKED);
    app_interface_set_flag(flag, enabled);
}

void app_interface_init(void)
{
    /* 绑定风扇开关。标志位初始为0，因此默认显示OFF。 */
    if(g_setting_ui.fan_switch != NULL) {
        lv_obj_add_event_cb(g_setting_ui.fan_switch,
                            app_interface_switch_event_cb,
                            LV_EVENT_VALUE_CHANGED,
                            (void *)(uintptr_t)APP_FLAG_FAN_ENABLED);
    }

    /* 绑定舵机开窗开关。标志位初始为0，因此默认显示CLOSED。 */
    if(g_setting_ui.window_switch != NULL) {
        lv_obj_add_event_cb(g_setting_ui.window_switch,
                            app_interface_switch_event_cb,
                            LV_EVENT_VALUE_CHANGED,
                            (void *)(uintptr_t)APP_FLAG_WINDOW_OPEN);
    }

    for(app_sensor_id_t sensor = APP_SENSOR_CO; sensor < APP_SENSOR_COUNT; sensor++) {
        app_interface_sync_sensor_led(sensor);
    }
    app_interface_sync_control_widgets();
}

/**
 * @brief 将STM32 RTC读出的数字格式化后写入两个Label。
 *
 * 本函数不维护时钟，也不创建LVGL定时器；RTC读取和刷新周期由STM32应用决定。
 * 典型做法是在LVGL任务中每分钟调用一次。
 */
void app_interface_set_datetime(uint16_t year,
                                uint8_t month,
                                uint8_t day,
                                uint8_t hour,
                                uint8_t minute)
{
    if(g_setting_ui.date_label != NULL) {
        lv_label_set_text_fmt(g_setting_ui.date_label,
                              "%04u-%02u-%02u",
                              (unsigned int)year,
                              (unsigned int)month,
                              (unsigned int)day);
    }
    if(g_setting_ui.time_label != NULL) {
        lv_label_set_text_fmt(g_setting_ui.time_label,
                              "%02u:%02u",
                              (unsigned int)hour,
                              (unsigned int)minute);
    }
}

/**
 * @brief 把温湿度传感器结果写入设置页。
 *
 * 显示固定保留一位小数，单位分别为C和%RH。
 */
void app_interface_set_environment(float temperature_c, float humidity_percent)
{
    if(g_setting_ui.temperature_label != NULL) {
        lv_label_set_text_fmt(g_setting_ui.temperature_label,
                              "Temp: %.1f C",
                              (double)temperature_c);
    }
    if(g_setting_ui.humidity_label != NULL) {
        lv_label_set_text_fmt(g_setting_ui.humidity_label,
                              "Humidity: %.1f %%RH",
                              (double)humidity_percent);
    }
}

/**
 * @brief 更新单路气体实时值。
 *
 * update_gauge_measurement()会根据page.c配置的阈值自动完成危险百分比计算和
 * 蓝色/黄色/红色切换，因此STM32只需要提供真实测量值。
 */
void app_interface_set_gas_value(app_sensor_id_t sensor, float measured_value)
{
    if(!app_sensor_id_valid(sensor)) {
        return;
    }
    update_gauge_measurement(&g_test_array[sensor], measured_value);
}

/** @brief 按固定枚举顺序批量更新六个实时仪表盘。 */
void app_interface_set_all_gas_values(const float values[APP_SENSOR_COUNT])
{
    if(values == NULL) {
        return;
    }
    for(app_sensor_id_t sensor = APP_SENSOR_CO; sensor < APP_SENSOR_COUNT; sensor++) {
        app_interface_set_gas_value(sensor, values[sensor]);
    }
}

/**
 * @brief 将某种气体完整的24小时数组交给对应历史图。
 *
 * Chart内部会自动绘制预警线、报警线，并根据每个数据段的等级选择颜色。
 */
void app_interface_set_history(app_sensor_id_t sensor,
                               const float values[APP_HISTORY_HOURS])
{
    if(!app_sensor_id_valid(sensor) || values == NULL) {
        return;
    }
    chart_set_24h_data(&g_chart_array[sensor], values);
}

/**
 * @brief 更新传感器连接标志。
 *
 * 该状态表示通信或传感器自检是否正常，不代表气体浓度是否安全。
 */
void app_interface_set_sensor_connected(app_sensor_id_t sensor, bool connected)
{
    if(!app_sensor_id_valid(sensor)) {
        return;
    }
    app_interface_set_flag(sensor_connection_flags[sensor], connected);
}

/**
 * @brief 修改一个标志位，并把变化同步到设置页。
 *
 * 对传感器标志会刷新对应LED；对风扇和舵机标志会刷新Switch与状态文字。
 */
void app_interface_set_flag(app_state_flag_t flag, bool enabled)
{
    /* 使用按位或置位，使用按位与加取反清零，不影响其他设备状态。 */
    if(enabled) {
        g_app_state_flags |= (uint32_t)flag;
    }
    else {
        g_app_state_flags &= ~((uint32_t)flag);
    }

    for(app_sensor_id_t sensor = APP_SENSOR_CO; sensor < APP_SENSOR_COUNT; sensor++) {
        if(flag == sensor_connection_flags[sensor]) {
            app_interface_sync_sensor_led(sensor);
            return;
        }
    }

    if(flag == APP_FLAG_FAN_ENABLED || flag == APP_FLAG_WINDOW_OPEN) {
        app_interface_sync_control_widgets();
    }
}

/** @brief 读取单个标志位，返回值可直接用于if判断。 */
bool app_interface_get_flag(app_state_flag_t flag)
{
    return (g_app_state_flags & (uint32_t)flag) != 0U;
}

/** @brief 返回完整位图，适合STM32执行器任务一次读取后分别判断。 */
uint32_t app_interface_get_flags(void)
{
    return g_app_state_flags;
}

/** @brief 程序侧设置风扇状态；与用户点击开关使用相同的标志同步路径。 */
void app_interface_set_fan_enabled(bool enabled)
{
    app_interface_set_flag(APP_FLAG_FAN_ENABLED, enabled);
}

/** @brief 程序侧设置窗户状态；true显示OPEN，false显示CLOSED。 */
void app_interface_set_window_open(bool open)
{
    app_interface_set_flag(APP_FLAG_WINDOW_OPEN, open);
}

/**
 * @brief 用一次数据快照刷新所有实时数值。
 *
 * 建议STM32传感器任务先填充app_display_snapshot_t，再把结构体发送给LVGL任务，
 * LVGL任务收到后调用本函数，从而避免界面读到一半新、一半旧的数据。
 */
void app_interface_update_snapshot(const app_display_snapshot_t *snapshot)
{
    if(snapshot == NULL) {
        return;
    }

    app_interface_set_datetime(snapshot->datetime.year,
                               snapshot->datetime.month,
                               snapshot->datetime.day,
                               snapshot->datetime.hour,
                               snapshot->datetime.minute);
    app_interface_set_all_gas_values(snapshot->gas_values);
    app_interface_set_environment(snapshot->temperature_c,
                                  snapshot->humidity_percent);
}
