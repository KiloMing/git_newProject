/**
 * @file testvalue.h
 * @brief 单个检测仪表盘组件的结构体和公共接口。
 */

#ifndef TESTVALUE_H
#define TESTVALUE_H

#include "lvgl.h"

/**
 * @brief 气体测量值的报警等级。
 *
 * 等级不仅决定仪表盘颜色，也用于把不同量纲的气体统一换算为0~100%的危险程度。
 */
typedef enum {
    TESTVALUE_LEVEL_NORMAL = 0, /**< 正常：蓝色。 */
    TESTVALUE_LEVEL_WARNING,    /**< 预警：黄色。 */
    TESTVALUE_LEVEL_ALARM       /**< 报警：红色。 */
} testvalue_level_t;

/** @brief 阈值模型：单边升高型，或O2使用的双边偏离型。 */
typedef enum {
    TESTVALUE_THRESHOLD_NONE = 0,
    TESTVALUE_THRESHOLD_HIGH,
    TESTVALUE_THRESHOLD_BAND
} testvalue_threshold_mode_t;

/**
 * @brief 单个检测仪表盘包含的全部对象和数据。
 *
 * 每个气体检测项都需要一个独立的testvalue_st实例，不能让多个
 * 仪表盘共用同一个实例，否则控件指针和测量值会互相覆盖。
 */
typedef struct {
    lv_obj_t *container;       /**< 网格单元中的外层容器。 */
    lv_obj_t *arc_value;       /**< 表示量程百分比的圆弧。 */
    lv_obj_t *label_value;     /**< 圆弧中央的百分比Label。 */
    lv_obj_t *label_text;      /**< 圆弧下方的实际测量值Label。 */
    float test_value;          /**< 当前量程百分比，范围为0～100。 */
    float measured_value;      /**< 当前传感器实际测量值。 */
    const char *name;          /**< 检测对象名称，例如"CO"。 */
    const char *unit;          /**< 测量单位，例如"ppm"或"%"。 */
    testvalue_level_t level;   /**< 当前正常、预警或报警等级。 */
    testvalue_threshold_mode_t threshold_mode; /**< 当前使用的阈值计算模型。 */
    float chart_min;           /**< 建议量程下限。 */
    float chart_max;           /**< 建议量程上限。 */
    float normal_low;          /**< 双边模型的正常下限。 */
    float normal_high;         /**< 正常上限；单边模型也使用此成员。 */
    float alarm_low;           /**< 双边模型的低浓度报警边界。 */
    float alarm_high;          /**< 报警上限；单边模型也使用此成员。 */
    bool alarm_inclusive;      /**< true表示达到阈值即报警，false表示超过阈值才报警。 */
} testvalue_st;

/**
 * @brief 创建一个完整的检测仪表盘组件。
 *
 * 创建结果由外层容器、圆弧、中央百分比和下方测量值文字组成。
 * 返回的结构体指针与传入的test_struct相同，便于调用方直接使用。
 *
 * @param test_struct   用于保存控件和数据的结构体，不能为空。
 * @param parent        仪表盘外层容器的父对象，通常是Grid。
 * @param name          检测对象名称，例如"CO"。
 * @param unit          实际测量值单位，例如"ppm"。
 * @param percent       初始量程百分比，函数内部限制为0～100。
 * @param measured_value 初始实际测量值，例如112 ppm。
 *
 * @return 成功时返回test_struct，参数无效时返回NULL。
 */
testvalue_st *testvalue_create(testvalue_st *test_struct,
                               lv_obj_t *parent,
                               const char *name,
                               const char *unit,
                               float percent,
                               float measured_value);

/**
 * @brief 更新一个检测仪表盘的百分比和实际测量值。
 *
 * 示例：
 * update_gauge_value(&g_test_array[0], 20.0f, 112.0f);
 * 中央显示"20%"，下方显示"CO(ppm): 112"。
 *
 * @param test_struct   需要更新的仪表盘结构体。
 * @param percent       新的量程百分比，函数内部限制为0～100。
 * @param measured_value 新的实际测量值。
 */
void update_gauge_value(testvalue_st *test_struct,
                        float percent,
                        float measured_value);

/**
 * @brief 配置浓度越高危险程度越高的气体阈值，例如CO、CO2、H2S、SO2和NH3。
 */
void testvalue_set_high_thresholds(testvalue_st *test_struct,
                                   float normal_max,
                                   float alarm_min,
                                   float chart_max,
                                   bool alarm_inclusive);

/**
 * @brief 配置低于或高于正常范围都会产生危险的阈值，主要用于O2。
 */
void testvalue_set_band_thresholds(testvalue_st *test_struct,
                                   float chart_min,
                                   float alarm_low,
                                   float normal_low,
                                   float normal_high,
                                   float alarm_high,
                                   float chart_max);

/**
 * @brief 传入新的真实测量值，自动计算危险百分比、报警等级和仪表盘颜色。
 */
void update_gauge_measurement(testvalue_st *test_struct, float measured_value);

#endif /* TESTVALUE_H */
