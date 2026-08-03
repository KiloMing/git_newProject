/**
 * @file ui.c
 * @brief UI总入口和模拟数据定时更新逻辑。
 *
 * 本文件只负责：
 * 1. 创建整个UI页面；
 * 2. 在Windows模拟器中周期性生成测试数据；
 * 3. 将新的百分比和实际测量值传递给仪表盘更新函数。
 *
 * 后续移植到STM32时，可以保留ui_init()，并将模拟定时器替换为
 * 传感器任务、消息队列或实际数据回调。
 */

#include "ui.h"

#include "page.h"
#include "testvalue.h"
#include "chart.h"

/*
 * PC模拟器可将本宏改为1，以启用自动变化的演示数据。
 * STM32工程保持为0，界面数值统一由app_interface.h中的接口写入。
 */
#ifndef UI_ENABLE_SIMULATOR_DATA
#define UI_ENABLE_SIMULATOR_DATA 0
#endif

/*
 * NT35510上板显示诊断：直接在活动屏幕顶层创建六个Arc。
 * 这些对象不经过Tab内容区、Grid或testvalue组件，用于排除父容器布局问题。
 */
#define UI_ENABLE_DIRECT_ARC_OVERLAY 1

#if UI_ENABLE_DIRECT_ARC_OVERLAY
static void ui_create_direct_arc_overlay(lv_obj_t *parent)
{
    static const char *names[6] = {"CO", "CO2", "O2", "H2S", "SO2", "NH3"};
    static const int32_t values[6] = {20, 30, 40, 50, 60, 70};
    static const int32_t x_pos[6] = {5, 265, 525, 5, 265, 525};
    static const int32_t y_pos[6] = {35, 35, 35, 250, 250, 250};
    int32_t i;

    for(i = 0; i < 6; i++) {
        lv_obj_t *container = lv_obj_create(parent);
        lv_obj_set_size(container, 250, 205);
        lv_obj_set_pos(container, x_pos[i], y_pos[i]);
        lv_obj_set_scrollable(container, false);
        lv_obj_set_style_bg_color(container, lv_color_hex(0xF5F5F5), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(container, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_color(container,
                                      lv_palette_main(LV_PALETTE_BLUE),
                                      LV_PART_MAIN);
        lv_obj_set_style_border_width(container, 2, LV_PART_MAIN);

        lv_obj_t *arc = lv_arc_create(container);
        lv_obj_set_size(arc, 140, 140);
        lv_obj_align(arc, LV_ALIGN_TOP_MID, 0, 5);
        lv_obj_set_clickable(arc, false);
        lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
        lv_arc_set_range(arc, 0, 100);
        lv_arc_set_rotation(arc, 135);
        lv_arc_set_bg_angles(arc, 0, 270);
        lv_arc_set_value(arc, values[i]);
        lv_obj_set_style_arc_width(arc, 14, LV_PART_MAIN);
        lv_obj_set_style_arc_color(arc, lv_color_hex(0xD0D0D0), LV_PART_MAIN);
        lv_obj_set_style_arc_opa(arc, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_arc_width(arc, 14, LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(arc,
                                   lv_palette_main(LV_PALETTE_BLUE),
                                   LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(arc, LV_OPA_COVER, LV_PART_INDICATOR);

        lv_obj_t *percent_label = lv_label_create(arc);
        lv_label_set_text_fmt(percent_label, "%ld%%", (long)values[i]);
        lv_obj_set_style_text_color(percent_label, lv_color_black(), LV_PART_MAIN);
        lv_obj_center(percent_label);

        lv_obj_t *name_label = lv_label_create(container);
        lv_label_set_text(name_label, names[i]);
        lv_obj_set_style_text_color(name_label, lv_color_black(), LV_PART_MAIN);
        lv_obj_align(name_label, LV_ALIGN_BOTTOM_MID, 0, -8);
    }
}
#endif

#if UI_ENABLE_SIMULATOR_DATA
/** 模拟数据刷新周期，单位为毫秒。 */
#define UI_UPDATE_PERIOD_MS 500

/**
 * @brief LVGL定时器回调函数，周期性更新六个检测仪表。
 *
 * 当前用于Windows环境下演示动态数据：
 * - 仪表盘百分比每次增加1%；
 * - 实际测量值每次增加1；
 * - 百分比超过100%后从0%重新开始。
 *
 * @param timer 触发本回调的LVGL定时器。本函数不需要使用该参数。
 */
static void ui_update_timer_cb(lv_timer_t *timer)
{
    /* 明确标记参数未使用，避免编译器产生警告。 */
    LV_UNUSED(timer);

    /* 依次更新CO、CO2、O2、NO2、SO2和H2S六个仪表盘。 */
    for(int i = 0; i < 6; i++) {
        /* 计算下一次显示的量程百分比。 */
        /* 危险百分比不再独立累加，而是由下面的真实测量值和气体阈值自动计算。 */

        /* 百分比限制在0～100，超过100后重新从0开始。 */
        /* 真实测量值到达建议量程上限后，从量程下限重新开始模拟。 */

        /*
         * 生成下一次模拟测量值。
         * 接入真实传感器后，这里应替换为传感器读取结果。
         */
        float measurement_step = (g_test_array[i].chart_max - g_test_array[i].chart_min) / 100.0f;
        float next_measurement = g_test_array[i].measured_value + measurement_step;
        if(next_measurement > g_test_array[i].chart_max) {
            next_measurement = g_test_array[i].chart_min;
        }

        /* 同时更新圆弧百分比、中央数值和下方实际测量值。 */
        update_gauge_measurement(&g_test_array[i], next_measurement);
    }
}
#endif /* UI_ENABLE_SIMULATOR_DATA */

/**
 * @brief 初始化完整UI。
 *
 * 调用本函数前必须已经完成lv_init()以及显示、输入设备初始化。
 * 本函数首先创建所有页面和仪表盘，然后创建周期性模拟数据定时器。
 */
void ui_init(void)
{
    /* 获取当前活动屏幕，所有页面都创建在该屏幕下。 */
    lv_obj_t *scr = lv_screen_active();

    /* 创建标签页、网格和六个检测仪表盘。 */
    page_create(scr);

#if UI_ENABLE_DIRECT_ARC_OVERLAY
    /* 直接显示层位于TabView之上，保证不受第一页父容器布局影响。 */
    ui_create_direct_arc_overlay(scr);
#endif

    /*
     * 使用LVGL定时器更新模拟检测值，避免使用阻塞循环，
     * 从而保证lv_timer_handler()能够持续刷新界面和处理输入。
     */
#if UI_ENABLE_SIMULATOR_DATA
    lv_timer_create(ui_update_timer_cb, UI_UPDATE_PERIOD_MS, NULL);
#endif
}
