/**
 ****************************************************************************************************
 * @file        lv_arc_demo.c
 * @brief       正点原子 LVGL例程10（lv_arc 圆弧）的 LVGL 9 适配版本
 *
 * @note
 * 1. 布局、尺寸计算和联动逻辑均沿用官方例程。
 * 2. 仅将 LVGL 8 接口替换为 LVGL 9 对应接口，例如 lv_scr_act() -> lv_screen_active()。
 * 3. 此页面用于单独验证 LCD方向、LVGL绘制和刷新链路，验证完成后可切回 ui_init()。
 ****************************************************************************************************
 */

#include "lv_arc_demo.h"
#include "lvgl.h"

#define ARC_DEMO_SCREEN_WIDTH()  lv_obj_get_width(lv_screen_active())
#define ARC_DEMO_SCREEN_HEIGHT() lv_obj_get_height(lv_screen_active())

static lv_obj_t *s_label_left;
static lv_obj_t *s_label_right;
static lv_obj_t *s_arc_left;
static lv_obj_t *s_arc_right;
static const lv_font_t *s_font;
static int32_t s_arc_width;

/** 左侧圆弧变化时，同步两个标签和右侧圆弧。 */
static void arc_demo_event_cb(lv_event_t *event)
{
    lv_obj_t *target = lv_event_get_target_obj(event);

    if(lv_event_get_code(event) == LV_EVENT_VALUE_CHANGED) {
        int32_t value = lv_arc_get_value(target);
        lv_label_set_text_fmt(s_label_left, "%d%%", (int)value);
        lv_label_set_text_fmt(s_label_right, "%d%%", (int)value);
        lv_arc_set_value(s_arc_right, value);
    }
}

static void arc_demo_create_left(void)
{
    int32_t screen_width = ARC_DEMO_SCREEN_WIDTH();
    int32_t screen_height = ARC_DEMO_SCREEN_HEIGHT();

    /* 与官方例程一致，根据实际屏幕宽度选择字体和圆弧线宽。 */
    if(screen_width <= 480) {
        s_font = &lv_font_montserrat_14;
        s_arc_width = 10;
    }
    else {
        s_font = &lv_font_montserrat_30;
        s_arc_width = 20;
    }

    s_arc_left = lv_arc_create(lv_screen_active());
    lv_obj_set_size(s_arc_left, screen_height * 3 / 8, screen_height * 3 / 8);
    lv_obj_align(s_arc_left, LV_ALIGN_CENTER, -screen_width / 5, 0);
    lv_arc_set_value(s_arc_left, 0);
    lv_obj_set_style_arc_width(s_arc_left, s_arc_width, LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_arc_left, s_arc_width, LV_PART_INDICATOR);
    lv_obj_add_event_cb(s_arc_left, arc_demo_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    s_label_left = lv_label_create(lv_screen_active());
    lv_obj_align(s_label_left, LV_ALIGN_CENTER, -screen_width / 5, 0);
    lv_label_set_text(s_label_left, "0%");
    lv_obj_set_style_text_font(s_label_left, s_font, LV_PART_MAIN);
}

static void arc_demo_create_right(void)
{
    int32_t screen_width = ARC_DEMO_SCREEN_WIDTH();
    int32_t screen_height = ARC_DEMO_SCREEN_HEIGHT();

    s_arc_right = lv_arc_create(lv_screen_active());
    lv_obj_set_size(s_arc_right, screen_height * 3 / 8, screen_height * 3 / 8);
    lv_obj_align(s_arc_right, LV_ALIGN_CENTER, screen_width / 5, 0);
    lv_arc_set_value(s_arc_right, 0);
    lv_arc_set_bg_angles(s_arc_right, 0, 360);
    lv_arc_set_rotation(s_arc_right, 270);
    lv_obj_remove_style(s_arc_right, NULL, LV_PART_KNOB);
    lv_obj_remove_flag(s_arc_right, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(s_arc_right, s_arc_width, LV_PART_MAIN);
    lv_obj_set_style_arc_width(s_arc_right, s_arc_width, LV_PART_INDICATOR);

    s_label_right = lv_label_create(lv_screen_active());
    lv_obj_align(s_label_right, LV_ALIGN_CENTER, screen_width / 5, 0);
    lv_label_set_text(s_label_right, "0%");
    lv_obj_set_style_text_font(s_label_right, s_font, LV_PART_MAIN);
}

void lv_arc_demo_create(void)
{
    /* 确保获取的是显示驱动注册后的真实屏幕尺寸。 */
    lv_obj_update_layout(lv_screen_active());
    arc_demo_create_left();
    arc_demo_create_right();
}
