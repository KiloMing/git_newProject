#ifndef __CHART_H
#define __CHART_H

#include "lvgl.h"
#include "testvalue.h"

#define CHART_HISTORY_HOURS 24
#define CHART_Y_LABEL_COUNT 6
#define CHART_MAX_THRESHOLD_LINES 4

/**
 * @brief 单个24小时历史图表所需的控件、数据和阈值信息。
 */
typedef struct {
    lv_obj_t *chart_container;   /**< 图表外层容器。 */
    lv_obj_t *chart_label;       /**< 图表标题。 */
    lv_obj_t *chart;             /**< LVGL折线图。 */
    lv_obj_t *x_scale;           /**< 00~23时的X轴。 */
    lv_obj_t *y_scale;           /**< 实际测量值的Y轴刻度。 */
    lv_chart_series_t *data_series; /**< 24小时实际数据系列。 */
    lv_chart_series_t *threshold_series[CHART_MAX_THRESHOLD_LINES];
    uint8_t threshold_series_count;
    const char *label_text;
    const testvalue_st *thresholds; /**< 与仪表盘共用的气体阈值。 */
    float data[CHART_HISTORY_HOURS];
    int32_t scale_factor;        /**< 小数数据转换为Chart整数的倍率。 */
    char y_label_text[CHART_Y_LABEL_COUNT][16];
    const char *y_label_src[CHART_Y_LABEL_COUNT + 1];
} chart_st;

/**
 * @brief 创建带阈值线、坐标轴和24小时模拟数据的历史图表。
 *
 * @param chart_data 保存图表控件和数据的结构体。
 * @param parent 图表父对象，当前为PastValues标签页。
 * @param label_text 图表标题。
 * @param data_array 依次表示00:00~23:00的24个测量值。
 * @param thresholds 对应仪表盘的阈值配置。
 */
chart_st *create_simple_chart(chart_st *chart_data,
                              lv_obj_t *parent,
                              const char *label_text,
                              const float data_array[CHART_HISTORY_HOURS],
                              const testvalue_st *thresholds);

/**
 * @brief 更新一张图表的完整24小时数据。
 */
void chart_set_24h_data(chart_st *chart_data,
                        const float data_array[CHART_HISTORY_HOURS]);

#endif /* __CHART_H */
