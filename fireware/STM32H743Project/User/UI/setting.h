#ifndef SETTING_H
#define SETTING_H

#include "lvgl.h"

#define SETTING_SENSOR_COUNT 6

/** @brief 设置页面中的六路气体传感器顺序。 */
typedef enum {
    SETTING_SENSOR_CO = 0,
    SETTING_SENSOR_CO2,
    SETTING_SENSOR_O2,
    SETTING_SENSOR_H2S,
    SETTING_SENSOR_SO2,
    SETTING_SENSOR_NH3
} setting_sensor_id_t;

/**
 * @brief 设置页面控件集合。
 *
 * 当前阶段只创建布局并保存控件指针。以后移植到STM32时，可通过这些指针更新
 * RTC时间、传感器连接状态、温湿度以及风扇和舵机的显示状态。
 */
typedef struct {
    lv_obj_t *root;

    lv_obj_t *date_label;                       /**< 年月日数字标签。 */
    lv_obj_t *time_label;                       /**< 时分数字标签。 */

    lv_obj_t *sensor_led[SETTING_SENSOR_COUNT]; /**< 绿色正常、红色错误。 */
    lv_obj_t *sensor_label[SETTING_SENSOR_COUNT];

    lv_obj_t *temperature_label;                /**< 当前温度数字标签。 */
    lv_obj_t *humidity_label;                   /**< 当前湿度数字标签。 */

    lv_obj_t *fan_switch;                       /**< 电机/风扇开关。 */
    lv_obj_t *fan_state_label;
    lv_obj_t *window_switch;                    /**< 舵机/窗户开关。 */
    lv_obj_t *window_state_label;
} setting_ui_st;

extern setting_ui_st g_setting_ui;

/** @brief 在Settings标签页中创建静态设置布局。 */
void setting_create(lv_obj_t *parent);

#endif /* SETTING_H */
