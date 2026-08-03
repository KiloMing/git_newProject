#include "chart.h"

/* X轴每3小时显示一个标签，图中仍然保留完整的24个小时数据点。 */
static const char *hour_labels[] = {
    "00", "03", "06", "09", "12", "15", "18", "21", NULL
};

/** @brief 判断数据点属于正常、预警还是报警状态。 */
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

    bool alarm_reached = thresholds->alarm_inclusive
                         ? value >= thresholds->alarm_high
                         : value > thresholds->alarm_high;
    return alarm_reached ? TESTVALUE_LEVEL_ALARM : TESTVALUE_LEVEL_WARNING;
}

/** @brief 返回等级对应的折线颜色。 */
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

/** @brief 将真实浮点测量值转换为LVGL Chart使用的整数。 */
static int32_t chart_scaled_value(const chart_st *chart_data, float value)
{
    return (int32_t)(value * (float)chart_data->scale_factor + (value >= 0.0f ? 0.5f : -0.5f));
}

/** @brief 添加一条贯穿24小时的水平阈值线。 */
static void chart_add_threshold_line(chart_st *chart_data, float value, lv_color_t color)
{
    if(chart_data->threshold_series_count >= CHART_MAX_THRESHOLD_LINES) {
        return;
    }

    lv_chart_series_t *series = lv_chart_add_series(chart_data->chart,
                                                    color,
                                                    LV_CHART_AXIS_PRIMARY_Y);
    chart_data->threshold_series[chart_data->threshold_series_count++] = series;

    int32_t scaled_value = chart_scaled_value(chart_data, value);
    for(uint32_t i = 0; i < CHART_HISTORY_HOURS; i++) {
        lv_chart_set_series_value_by_id(chart_data->chart, series, i, scaled_value);
    }
}

/** @brief 创建Y轴的真实数值标签，避免内部10倍缩放影响显示。 */
static void chart_build_y_labels(chart_st *chart_data)
{
    float range = chart_data->thresholds->chart_max - chart_data->thresholds->chart_min;
    for(uint32_t i = 0; i < CHART_Y_LABEL_COUNT; i++) {
        float value = chart_data->thresholds->chart_min
                      + range * (float)i / (float)(CHART_Y_LABEL_COUNT - 1);
        int32_t scaled = chart_scaled_value(chart_data, value);
        if(chart_data->scale_factor == 10) {
            lv_snprintf(chart_data->y_label_text[i],
                        sizeof(chart_data->y_label_text[i]),
                        "%" LV_PRId32 ".%" LV_PRId32,
                        scaled / 10,
                        LV_ABS(scaled % 10));
        }
        else {
            lv_snprintf(chart_data->y_label_text[i],
                        sizeof(chart_data->y_label_text[i]),
                        "%" LV_PRId32,
                        scaled);
        }
        chart_data->y_label_src[i] = chart_data->y_label_text[i];
    }
    chart_data->y_label_src[CHART_Y_LABEL_COUNT] = NULL;
}

/**
 * @brief 将蓝色原始数据折线按每一段的实际等级重新绘制。
 *
 * 阈值线本身是黄色或红色，不会进入该回调分支。每一段数据线取两个端点中
 * 更严重的等级，因此跨越阈值的线段能够立即显示为黄色或红色。
 */
static void chart_draw_event_cb(lv_event_t *event)
{
    lv_draw_task_t *task = lv_event_get_draw_task(event);
    lv_draw_dsc_base_t *base = (lv_draw_dsc_base_t *)lv_draw_task_get_draw_dsc(task);
    chart_st *chart_data = (chart_st *)lv_event_get_user_data(event);

    if(base == NULL || chart_data == NULL || base->part != LV_PART_ITEMS ||
       lv_draw_task_get_type(task) != LV_DRAW_TASK_TYPE_LINE) {
        return;
    }

    lv_draw_line_dsc_t *line = lv_draw_task_get_line_dsc(task);
    if(line == NULL || line->point_cnt < 2) {
        return;
    }

    /* 黄色和红色系列是阈值线，使用虚线与实际历史曲线区分。 */
    if(lv_color_eq(line->color, lv_palette_main(LV_PALETTE_YELLOW)) ||
       lv_color_eq(line->color, lv_palette_main(LV_PALETTE_RED))) {
        line->dash_width = 6;
        line->dash_gap = 4;
        line->width = 2;
        return;
    }

    if(!lv_color_eq(line->color, lv_palette_main(LV_PALETTE_BLUE))) {
        return;
    }

    /* 隐藏LVGL生成的单色数据线，随后逐段绘制对应等级颜色。 */
    line->opa = LV_OPA_TRANSP;
    for(int32_t i = 0; i < line->point_cnt - 1 && i < CHART_HISTORY_HOURS - 1; i++) {
        if(line->points[i].x == LV_DRAW_LINE_POINT_NONE ||
           line->points[i + 1].x == LV_DRAW_LINE_POINT_NONE) {
            continue;
        }

        testvalue_level_t level1 = chart_get_level(chart_data->thresholds, chart_data->data[i]);
        testvalue_level_t level2 = chart_get_level(chart_data->thresholds, chart_data->data[i + 1]);
        testvalue_level_t segment_level = level1 > level2 ? level1 : level2;

        lv_draw_line_dsc_t segment;
        lv_draw_line_dsc_init(&segment);
        segment.p1 = line->points[i];
        segment.p2 = line->points[i + 1];
        segment.color = chart_level_color(segment_level);
        segment.width = 3;
        segment.opa = LV_OPA_COVER;
        segment.round_start = 1;
        segment.round_end = 1;
        lv_draw_line(base->layer, &segment);
    }
}

void chart_set_24h_data(chart_st *chart_data,
                        const float data_array[CHART_HISTORY_HOURS])
{
    if(chart_data == NULL || chart_data->chart == NULL ||
       chart_data->data_series == NULL || data_array == NULL) {
        return;
    }

    for(uint32_t i = 0; i < CHART_HISTORY_HOURS; i++) {
        chart_data->data[i] = data_array[i];
        lv_chart_set_series_value_by_id(chart_data->chart,
                                        chart_data->data_series,
                                        i,
                                        chart_scaled_value(chart_data, data_array[i]));
    }
    lv_chart_refresh(chart_data->chart);
}

chart_st *create_simple_chart(chart_st *chart_data,
                              lv_obj_t *parent,
                              const char *label_text,
                              const float data_array[CHART_HISTORY_HOURS],
                              const testvalue_st *thresholds)
{
    if(chart_data == NULL || parent == NULL || label_text == NULL ||
       data_array == NULL || thresholds == NULL) {
        return NULL;
    }

    *chart_data = (chart_st){0};
    chart_data->label_text = label_text;
    chart_data->thresholds = thresholds;
    chart_data->scale_factor = thresholds->chart_max <= 25.0f ? 10 : 1;
    chart_build_y_labels(chart_data);

    /* 使用Grid让标题、Y轴、图表和X轴保持严格对齐。 */
    static int32_t col_dsc[] = {52, LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static int32_t row_dsc[] = {22, 145, 30, LV_GRID_TEMPLATE_LAST};

    chart_data->chart_container = lv_obj_create(parent);
    lv_obj_set_size(chart_data->chart_container, 430, 205);
    lv_obj_set_grid_dsc_array(chart_data->chart_container, col_dsc, row_dsc);
    lv_obj_set_style_pad_all(chart_data->chart_container, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_row(chart_data->chart_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(chart_data->chart_container, 0, LV_PART_MAIN);
    lv_obj_set_scrollable(chart_data->chart_container, false);

    chart_data->chart_label = lv_label_create(chart_data->chart_container);
    lv_label_set_text(chart_data->chart_label, label_text);
    lv_obj_set_grid_cell(chart_data->chart_label,
                         LV_GRID_ALIGN_CENTER, 0, 2,
                         LV_GRID_ALIGN_CENTER, 0, 1);

    chart_data->y_scale = lv_scale_create(chart_data->chart_container);
    lv_scale_set_mode(chart_data->y_scale, LV_SCALE_MODE_VERTICAL_LEFT);
    lv_scale_set_range(chart_data->y_scale,
                       chart_scaled_value(chart_data, thresholds->chart_min),
                       chart_scaled_value(chart_data, thresholds->chart_max));
    lv_scale_set_total_tick_count(chart_data->y_scale, CHART_Y_LABEL_COUNT);
    lv_scale_set_major_tick_every(chart_data->y_scale, 1);
    lv_scale_set_text_src(chart_data->y_scale, chart_data->y_label_src);
    lv_obj_set_style_text_font(chart_data->y_scale, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_grid_cell(chart_data->y_scale,
                         LV_GRID_ALIGN_END, 0, 1,
                         LV_GRID_ALIGN_STRETCH, 1, 1);

    chart_data->chart = lv_chart_create(chart_data->chart_container);
    lv_chart_set_type(chart_data->chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(chart_data->chart, CHART_HISTORY_HOURS);
    lv_chart_set_axis_min_value(chart_data->chart,
                                LV_CHART_AXIS_PRIMARY_Y,
                                chart_scaled_value(chart_data, thresholds->chart_min));
    lv_chart_set_axis_max_value(chart_data->chart,
                                LV_CHART_AXIS_PRIMARY_Y,
                                chart_scaled_value(chart_data, thresholds->chart_max));
    lv_chart_set_div_line_count(chart_data->chart, CHART_Y_LABEL_COUNT, 9);
    lv_obj_set_style_size(chart_data->chart, 0, 0, LV_PART_INDICATOR);
    lv_obj_set_scrollable(chart_data->chart, false);
    lv_obj_set_grid_cell(chart_data->chart,
                         LV_GRID_ALIGN_STRETCH, 1, 1,
                         LV_GRID_ALIGN_STRETCH, 1, 1);

    /* 单边气体有一条黄线和一条红线；O2上下两侧各有两条。 */
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
                        LV_EVENT_DRAW_TASK_ADDED,
                        chart_data);
    lv_obj_set_send_draw_task_events(chart_data->chart, true);

    chart_data->x_scale = lv_scale_create(chart_data->chart_container);
    lv_scale_set_mode(chart_data->x_scale, LV_SCALE_MODE_HORIZONTAL_BOTTOM);
    lv_scale_set_total_tick_count(chart_data->x_scale, CHART_HISTORY_HOURS);
    lv_scale_set_major_tick_every(chart_data->x_scale, 3);
    lv_scale_set_text_src(chart_data->x_scale, hour_labels);
    lv_obj_set_style_text_font(chart_data->x_scale, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_grid_cell(chart_data->x_scale,
                         LV_GRID_ALIGN_STRETCH, 1, 1,
                         LV_GRID_ALIGN_START, 2, 1);

    chart_set_24h_data(chart_data, data_array);
    return chart_data;
}
