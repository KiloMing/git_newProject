#include "chart.h"

/** 根据测量值判断当前属于正常、警告还是报警状态。 */
static testvalue_level_t chart_get_level(const testvalue_st *thresholds, float value)
{
    if(thresholds->threshold_mode == TESTVALUE_THRESHOLD_BAND) {
        if(value >= thresholds->normal_low && value <= thresholds->normal_high) {
            return TESTVALUE_LEVEL_NORMAL;
        }
        if(value >= thresholds->alarm_low && value <= thresholds->alarm_high) {
            return TESTVALUE_LEVEL_WARNING;
        }
        return TESTVALUE_LEVEL_ALARM;
    }

    if(value <= thresholds->normal_high) {
        return TESTVALUE_LEVEL_NORMAL;
    }

    if(thresholds->alarm_inclusive) {
        return value >= thresholds->alarm_high ? TESTVALUE_LEVEL_ALARM : TESTVALUE_LEVEL_WARNING;
    }
    return value > thresholds->alarm_high ? TESTVALUE_LEVEL_ALARM : TESTVALUE_LEVEL_WARNING;
}

/** 返回状态对应的折线颜色：正常蓝、警告黄、报警红。 */
static lv_color_t chart_level_color(testvalue_level_t level)
{
    if(level == TESTVALUE_LEVEL_ALARM) {
        return lv_palette_main(LV_PALETTE_RED);
    }
    if(level == TESTVALUE_LEVEL_WARNING) {
        return lv_palette_main(LV_PALETTE_YELLOW);
    }
    return lv_palette_main(LV_PALETTE_BLUE);
}

/** 将浮点测量值转换为LVGL8 Chart使用的整数。 */
static int32_t chart_scaled_value(const chart_st *chart_data, float value)
{
    float scaled = value * (float)chart_data->scale_factor;
    return (int32_t)(scaled + (scaled >= 0.0f ? 0.5f : -0.5f));
}

/** 添加贯穿24小时的水平阈值线。 */
static void chart_add_threshold_line(chart_st *chart_data, float value, lv_color_t color)
{
    lv_chart_series_t *series;
    uint32_t i;

    if(chart_data->threshold_series_count >= CHART_MAX_THRESHOLD_LINES) {
        return;
    }

    series = lv_chart_add_series(chart_data->chart, color, LV_CHART_AXIS_PRIMARY_Y);
    if(series == NULL) {
        return;
    }

    chart_data->threshold_series[chart_data->threshold_series_count++] = series;
    for(i = 0; i < CHART_HISTORY_HOURS; i++) {
        lv_chart_set_value_by_id(chart_data->chart,
                                 series,
                                 (uint16_t)i,
                                 (lv_coord_t)chart_scaled_value(chart_data, value));
    }
}

/**
 * LVGL8图表绘制回调。
 *
 * - X轴按照24小时制显示00~23；
 * - 小量程气体的Y轴恢复一位小数；
 * - 阈值线使用虚线；
 * - 每段历史折线根据两个端点中更危险的等级着色。
 */
static void chart_draw_event_cb(lv_event_t *event)
{
    chart_st *chart_data = (chart_st *)lv_event_get_user_data(event);
    lv_obj_draw_part_dsc_t *draw = lv_event_get_draw_part_dsc(event);

    if(chart_data == NULL || draw == NULL) {
        return;
    }

    if(draw->type == LV_CHART_DRAW_PART_TICK_LABEL && draw->text != NULL) {
        if(draw->id == LV_CHART_AXIS_PRIMARY_X) {
            lv_snprintf(draw->text, draw->text_length, "%02ld", (long)draw->value);
        }
        else if(draw->id == LV_CHART_AXIS_PRIMARY_Y && chart_data->scale_factor == 10) {
            int32_t value = draw->value;
            lv_snprintf(draw->text,
                        draw->text_length,
                        "%ld.%ld",
                        (long)(value / 10),
                        (long)LV_ABS(value % 10));
        }
        return;
    }

    if(draw->type != LV_CHART_DRAW_PART_LINE_AND_POINT || draw->line_dsc == NULL) {
        return;
    }

    if(draw->sub_part_ptr == chart_data->data_series) {
        uint32_t point = draw->id;
        if(point < CHART_HISTORY_HOURS - 1U) {
            testvalue_level_t level1 = chart_get_level(chart_data->thresholds, chart_data->data[point]);
            testvalue_level_t level2 = chart_get_level(chart_data->thresholds, chart_data->data[point + 1U]);
            testvalue_level_t segment_level = level1 > level2 ? level1 : level2;
            draw->line_dsc->color = chart_level_color(segment_level);
            draw->line_dsc->width = 3;
        }
        return;
    }

    /* 其余折线均为警告/报警阈值线。 */
    draw->line_dsc->dash_width = 6;
    draw->line_dsc->dash_gap = 4;
    draw->line_dsc->width = 2;
}

void chart_set_24h_data(chart_st *chart_data,
                        const float data_array[CHART_HISTORY_HOURS])
{
    uint32_t i;

    if(chart_data == NULL || chart_data->chart == NULL ||
       chart_data->data_series == NULL || data_array == NULL) {
        return;
    }

    for(i = 0; i < CHART_HISTORY_HOURS; i++) {
        chart_data->data[i] = data_array[i];
        lv_chart_set_value_by_id(chart_data->chart,
                                 chart_data->data_series,
                                 (uint16_t)i,
                                 (lv_coord_t)chart_scaled_value(chart_data, data_array[i]));
    }
    lv_chart_refresh(chart_data->chart);
}

chart_st *create_simple_chart(chart_st *chart_data,
                              lv_obj_t *parent,
                              const char *label_text,
                              const float data_array[CHART_HISTORY_HOURS],
                              const testvalue_st *thresholds)
{
    int32_t y_min;
    int32_t y_max;

    if(chart_data == NULL || parent == NULL || label_text == NULL ||
       data_array == NULL || thresholds == NULL) {
        return NULL;
    }

    *chart_data = (chart_st){0};
    chart_data->label_text = label_text;
    chart_data->thresholds = thresholds;
    chart_data->scale_factor = thresholds->chart_max <= 25.0f ? 10 : 1;
    y_min = chart_scaled_value(chart_data, thresholds->chart_min);
    y_max = chart_scaled_value(chart_data, thresholds->chart_max);

    chart_data->chart_container = lv_obj_create(parent);
    lv_obj_set_size(chart_data->chart_container, 430, 205);
    lv_obj_clear_flag(chart_data->chart_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(chart_data->chart_container, 0, LV_PART_MAIN);

    chart_data->chart_label = lv_label_create(chart_data->chart_container);
    lv_label_set_text(chart_data->chart_label, label_text);
    lv_obj_align(chart_data->chart_label, LV_ALIGN_TOP_MID, 0, 2);

    chart_data->chart = lv_chart_create(chart_data->chart_container);
    lv_obj_set_pos(chart_data->chart, 48, 25);
    lv_obj_set_size(chart_data->chart, 370, 155);
    lv_obj_clear_flag(chart_data->chart, LV_OBJ_FLAG_SCROLLABLE);
    lv_chart_set_type(chart_data->chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(chart_data->chart, CHART_HISTORY_HOURS);
    lv_chart_set_range(chart_data->chart, LV_CHART_AXIS_PRIMARY_Y, y_min, y_max);
    lv_chart_set_range(chart_data->chart, LV_CHART_AXIS_PRIMARY_X, 0, 23);
    lv_chart_set_div_line_count(chart_data->chart, CHART_Y_LABEL_COUNT, 9);
    lv_chart_set_axis_tick(chart_data->chart,
                           LV_CHART_AXIS_PRIMARY_Y,
                           4, 2, CHART_Y_LABEL_COUNT, 1, true, 42);
    lv_chart_set_axis_tick(chart_data->chart,
                           LV_CHART_AXIS_PRIMARY_X,
                           4, 2, 9, 3, true, 20);
    lv_obj_set_style_size(chart_data->chart, 0, LV_PART_INDICATOR);

    if(thresholds->threshold_mode == TESTVALUE_THRESHOLD_BAND) {
        chart_add_threshold_line(chart_data, thresholds->normal_low, lv_palette_main(LV_PALETTE_YELLOW));
        chart_add_threshold_line(chart_data, thresholds->normal_high, lv_palette_main(LV_PALETTE_YELLOW));
        chart_add_threshold_line(chart_data, thresholds->alarm_low, lv_palette_main(LV_PALETTE_RED));
        chart_add_threshold_line(chart_data, thresholds->alarm_high, lv_palette_main(LV_PALETTE_RED));
    }
    else {
        chart_add_threshold_line(chart_data, thresholds->normal_high, lv_palette_main(LV_PALETTE_YELLOW));
        chart_add_threshold_line(chart_data, thresholds->alarm_high, lv_palette_main(LV_PALETTE_RED));
    }

    chart_data->data_series = lv_chart_add_series(chart_data->chart,
                                                  lv_palette_main(LV_PALETTE_BLUE),
                                                  LV_CHART_AXIS_PRIMARY_Y);
    lv_obj_add_event_cb(chart_data->chart,
                        chart_draw_event_cb,
                        LV_EVENT_DRAW_PART_BEGIN,
                        chart_data);

    chart_set_24h_data(chart_data, data_array);
    return chart_data;
}
