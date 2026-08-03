/**
 * @file testvalue.c
 * @brief 单个检测仪表盘组件的创建与更新实现。
 */

#include "testvalue.h"

/**
 * @brief 将百分比限制在仪表盘支持的0～100范围内。
 *
 * @param value 待检查的百分比。
 * @return 限制后的百分比。
 */
static float clamp_percent(float value)
{
    if(value < 0.0f) {
        return 0.0f;
    }
    if(value > 100.0f) {
        return 100.0f;
    }
    return value;
}

/** @brief 将一个数值区间线性映射到另一个区间，并限制在目标范围内。 */
static float map_segment(float value,
                         float input_min,
                         float input_max,
                         float output_min,
                         float output_max)
{
    if(input_max <= input_min) {
        return output_min;
    }

    float ratio = (value - input_min) / (input_max - input_min);
    ratio = clamp_percent(ratio * 100.0f) / 100.0f;
    return output_min + ratio * (output_max - output_min);
}

/** @brief 根据当前等级设置圆弧指示部分的颜色。 */
static void apply_level_color(testvalue_st *test_struct)
{
    lv_color_t color;

    if(test_struct->level == TESTVALUE_LEVEL_ALARM) {
        color = lv_palette_main(LV_PALETTE_RED);
    }
    else if(test_struct->level == TESTVALUE_LEVEL_WARNING) {
        color = lv_palette_main(LV_PALETTE_YELLOW);
    }
    else {
        color = lv_palette_main(LV_PALETTE_BLUE);
    }

    lv_obj_set_style_arc_color(test_struct->arc_value, color, LV_PART_INDICATOR);
}

/** @brief 计算单边升高型气体的危险百分比和等级。 */
static float calculate_high_risk(testvalue_st *test_struct, float measured_value)
{
    if(measured_value <= test_struct->normal_high) {
        test_struct->level = TESTVALUE_LEVEL_NORMAL;
        return map_segment(measured_value,
                           test_struct->chart_min,
                           test_struct->normal_high,
                           0.0f,
                           33.0f);
    }

    bool alarm_reached = test_struct->alarm_inclusive
                         ? measured_value >= test_struct->alarm_high
                         : measured_value > test_struct->alarm_high;
    if(!alarm_reached) {
        test_struct->level = TESTVALUE_LEVEL_WARNING;
        return map_segment(measured_value,
                           test_struct->normal_high,
                           test_struct->alarm_high,
                           33.0f,
                           66.0f);
    }

    test_struct->level = TESTVALUE_LEVEL_ALARM;
    if(test_struct->chart_max <= test_struct->alarm_high) {
        return 100.0f;
    }
    return map_segment(measured_value,
                       test_struct->alarm_high,
                       test_struct->chart_max,
                       66.0f,
                       100.0f);
}

/** @brief 计算O2这类双边偏离型测量值的危险百分比和等级。 */
static float calculate_band_risk(testvalue_st *test_struct, float measured_value)
{
    float center = (test_struct->normal_low + test_struct->normal_high) / 2.0f;

    if(measured_value >= test_struct->normal_low && measured_value <= test_struct->normal_high) {
        test_struct->level = TESTVALUE_LEVEL_NORMAL;
        if(measured_value <= center) {
            return map_segment(center - measured_value,
                               0.0f,
                               center - test_struct->normal_low,
                               0.0f,
                               33.0f);
        }
        return map_segment(measured_value - center,
                           0.0f,
                           test_struct->normal_high - center,
                           0.0f,
                           33.0f);
    }

    if(measured_value >= test_struct->alarm_low && measured_value <= test_struct->alarm_high) {
        test_struct->level = TESTVALUE_LEVEL_WARNING;
        if(measured_value < test_struct->normal_low) {
            return map_segment(test_struct->normal_low - measured_value,
                               0.0f,
                               test_struct->normal_low - test_struct->alarm_low,
                               33.0f,
                               66.0f);
        }
        return map_segment(measured_value - test_struct->normal_high,
                           0.0f,
                           test_struct->alarm_high - test_struct->normal_high,
                           33.0f,
                           66.0f);
    }

    test_struct->level = TESTVALUE_LEVEL_ALARM;
    if(measured_value < test_struct->alarm_low) {
        return map_segment(test_struct->alarm_low - measured_value,
                           0.0f,
                           test_struct->alarm_low - test_struct->chart_min,
                           66.0f,
                           100.0f);
    }
    return map_segment(measured_value - test_struct->alarm_high,
                       0.0f,
                       test_struct->chart_max - test_struct->alarm_high,
                       66.0f,
                       100.0f);
}

/**
 * @brief 更新指定仪表盘的显示内容。
 *
 * 本函数会同步更新：
 * 1. 结构体中保存的百分比和实际测量值；
 * 2. 仪表盘圆弧；
 * 3. 圆弧中央的百分比文字；
 * 4. 圆弧下方的名称、单位和实际测量值。
 *
 * @param test_struct    需要更新的仪表盘结构体。
 * @param percent        新的量程百分比，允许输入任意值，内部会限制到0～100。
 * @param measured_value 新的实际测量值，不在本函数中改变单位或进行缩放。
 */
void update_gauge_value(testvalue_st *test_struct,
                        float percent,
                        float measured_value)
{
    /* 控件尚未创建或指针无效时直接返回，避免访问空指针。 */
    if(test_struct == NULL || test_struct->arc_value == NULL ||
       test_struct->label_value == NULL || test_struct->label_text == NULL) {
        return;
    }

    /* 圆弧量程固定为0～100，因此先限制传入的百分比。 */
    percent = clamp_percent(percent);

    /* 保存最新数据，供定时器或其他模块下一次更新时使用。 */
    test_struct->test_value = percent;
    test_struct->measured_value = measured_value;

    /* 根据已经计算出的等级更新指示圆弧颜色。 */
    apply_level_color(test_struct);

    /* 1. 更新圆弧对应的量程百分比。 */
    lv_arc_set_value(test_struct->arc_value, (int32_t)percent);

    /* 2. 更新圆弧中央文字，例如"20%"。 */
    lv_label_set_text_fmt(test_struct->label_value, "%.0f%%", percent);

    /*
     * 3. 更新圆弧下方的实际测量信息。
     * %g可以避免给整数强制添加无意义的小数，例如112显示为112；
     * 同时仍然能够显示20.9、0.03等带小数的测量值。
     */
    lv_label_set_text_fmt(test_struct->label_text,
                          "%s(%s): %g",
                          test_struct->name,
                          test_struct->unit,
                          measured_value);
}

void testvalue_set_high_thresholds(testvalue_st *test_struct,
                                   float normal_max,
                                   float alarm_min,
                                   float chart_max,
                                   bool alarm_inclusive)
{
    if(test_struct == NULL || normal_max < 0.0f || alarm_min <= normal_max || chart_max < alarm_min) {
        return;
    }

    test_struct->threshold_mode = TESTVALUE_THRESHOLD_HIGH;
    test_struct->chart_min = 0.0f;
    test_struct->chart_max = chart_max;
    test_struct->normal_high = normal_max;
    test_struct->alarm_high = alarm_min;
    test_struct->alarm_inclusive = alarm_inclusive;
    update_gauge_measurement(test_struct, test_struct->measured_value);
}

void testvalue_set_band_thresholds(testvalue_st *test_struct,
                                   float chart_min,
                                   float alarm_low,
                                   float normal_low,
                                   float normal_high,
                                   float alarm_high,
                                   float chart_max)
{
    if(test_struct == NULL || chart_min >= alarm_low || alarm_low >= normal_low ||
       normal_low >= normal_high || normal_high >= alarm_high || alarm_high >= chart_max) {
        return;
    }

    test_struct->threshold_mode = TESTVALUE_THRESHOLD_BAND;
    test_struct->chart_min = chart_min;
    test_struct->alarm_low = alarm_low;
    test_struct->normal_low = normal_low;
    test_struct->normal_high = normal_high;
    test_struct->alarm_high = alarm_high;
    test_struct->chart_max = chart_max;
    update_gauge_measurement(test_struct, test_struct->measured_value);
}

void update_gauge_measurement(testvalue_st *test_struct, float measured_value)
{
    if(test_struct == NULL) {
        return;
    }

    float danger_percent = test_struct->test_value;
    if(test_struct->threshold_mode == TESTVALUE_THRESHOLD_HIGH) {
        danger_percent = calculate_high_risk(test_struct, measured_value);
    }
    else if(test_struct->threshold_mode == TESTVALUE_THRESHOLD_BAND) {
        danger_percent = calculate_band_risk(test_struct, measured_value);
    }

    update_gauge_value(test_struct, danger_percent, measured_value);
}

/**
 * @brief 创建一个检测仪表盘组件。
 *
 * 组件层级如下：
 * container
 * ├─ arc_value
 * │  └─ label_value
 * └─ label_text
 *
 * @param test_struct    保存组件对象和数据的结构体。
 * @param parent         外层容器的父对象，通常为Grid。
 * @param name           检测对象名称。
 * @param unit           测量单位。
 * @param percent        初始量程百分比。
 * @param measured_value 初始实际测量值。
 * @return 成功时返回test_struct，失败时返回NULL。
 */
testvalue_st *testvalue_create(testvalue_st *test_struct,
                               lv_obj_t *parent,
                               const char *name,
                               const char *unit,
                               float percent,
                               float measured_value)
{
    int32_t display_width;
    int32_t display_height;
    int32_t arc_size;

    /* 0. 检查创建组件所需的输入参数。 */
    if(test_struct == NULL || parent == NULL || name == NULL || unit == NULL) {
        return NULL;
    }

    /* 1. 清空旧状态，防止结构体中残留无效控件指针。 */
    *test_struct = (testvalue_st){0};

    /* 2. 保存名称和单位。传入字符串应在仪表盘整个生命周期内有效。 */
    test_struct->name = name;
    test_struct->unit = unit;

    /* 3. 创建外层容器。page.c会将该容器放入对应Grid单元格。 */
    test_struct->container = lv_obj_create(parent);

    /*
     * 开发板可能连接320x240、480x320或800x480的MCU屏。
     * 去掉容器内部多余留白，让较小的320x240屏也能容纳三列两行仪表盘。
     */
    lv_obj_set_style_pad_all(test_struct->container, 0, LV_PART_MAIN);
    lv_obj_set_scrollable(test_struct->container, false);
    lv_obj_set_style_bg_color(test_struct->container,
                              lv_color_hex(0xF5F5F5),
                              LV_PART_MAIN);
    lv_obj_set_style_bg_opa(test_struct->container, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(test_struct->container,
                                  lv_palette_main(LV_PALETTE_BLUE),
                                  LV_PART_MAIN);
    lv_obj_set_style_border_width(test_struct->container, 2, LV_PART_MAIN);

    /* 4. 在外层容器中创建圆弧仪表盘。 */
    test_struct->arc_value = lv_arc_create(test_struct->container);

    /* 仪表盘仅用于显示，不允许用户通过鼠标拖动改变数值。 */
    lv_obj_set_clickable(test_struct->arc_value, false);

    /*
     * 根据LCD驱动注册到LVGL的真实分辨率选择仪表盘尺寸：
     * 320x240使用72像素，480x320使用100像素，800x480使用130像素。
     */
    display_width = lv_display_get_horizontal_resolution(NULL);
    display_height = lv_display_get_vertical_resolution(NULL);
    if(display_width <= 320 || display_height <= 240) {
        arc_size = 72;
    }
    else if(display_width >= 800 && display_height >= 480) {
        arc_size = 130;
    }
    else {
        arc_size = 100;
    }

    /* 设置圆弧尺寸和数值范围。 */
    lv_obj_set_size(test_struct->arc_value, arc_size, arc_size);
    lv_arc_set_range(test_struct->arc_value, 0, 100);

    /*
     * 显式设置圆弧样式，不依赖默认主题。
     * MAIN是灰色完整量程，INDICATOR由报警等级函数切换为蓝/黄/红。
     */
    lv_obj_set_style_arc_width(test_struct->arc_value, 12, LV_PART_MAIN);
    lv_obj_set_style_arc_color(test_struct->arc_value,
                               lv_color_hex(0xD9D9D9),
                               LV_PART_MAIN);
    lv_obj_set_style_arc_opa(test_struct->arc_value, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_arc_width(test_struct->arc_value, 12, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(test_struct->arc_value, LV_OPA_COVER, LV_PART_INDICATOR);

    /* 设置圆弧起始方向和270度显示范围，底部保留90度缺口。 */
    lv_arc_set_rotation(test_struct->arc_value, 135);
    lv_arc_set_bg_angles(test_struct->arc_value, 0, 270);

    /* 圆弧向上偏移，为下方测量文字预留空间。 */
    lv_obj_align(test_struct->arc_value,
                 LV_ALIGN_CENTER,
                 0,
                 (arc_size <= 72) ? -7 : -10);

    /* 隐藏圆弧旋钮，仅保留背景弧和指示弧。 */
    lv_obj_remove_style(test_struct->arc_value, NULL, LV_PART_KNOB);

    /* 5. 创建圆弧中央的百分比Label。 */
    test_struct->label_value = lv_label_create(test_struct->arc_value);
    lv_obj_set_style_text_color(test_struct->label_value,
                                lv_color_black(),
                                LV_PART_MAIN);
    lv_obj_center(test_struct->label_value);

    /* 6. 创建圆弧下方的实际测量值Label。 */
    test_struct->label_text = lv_label_create(test_struct->container);

    /*
     * Label宽度始终跟随当前网格单元，文字在自己的单元格内居中。
     * 原来的x=-35会让第一列文字越过左边界，也会使后两列互相重叠。
     */
    lv_obj_set_width(test_struct->label_text, LV_PCT(100));
    lv_obj_set_style_text_align(test_struct->label_text, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_color(test_struct->label_text,
                                lv_color_black(),
                                LV_PART_MAIN);
    lv_label_set_long_mode(test_struct->label_text, LV_LABEL_LONG_DOT);
    if(arc_size <= 72) {
        lv_obj_set_style_text_font(test_struct->label_text,
                                   &lv_font_montserrat_12,
                                   LV_PART_MAIN);
    }
    lv_obj_align_to(test_struct->label_text,
                    test_struct->arc_value,
                    LV_ALIGN_OUT_BOTTOM_MID,
                    0,
                    (arc_size <= 72) ? 0 : 2);

    /* 7. 使用统一更新函数设置初始圆弧、百分比和实际测量值。 */
    update_gauge_value(test_struct, percent, measured_value);

    return test_struct;
}
