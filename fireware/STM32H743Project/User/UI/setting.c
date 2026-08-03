#include "setting.h"

setting_ui_st g_setting_ui = {0};

/** 六路传感器名称与CurrentValues页面保持相同顺序。 */
static const char *sensor_names[SETTING_SENSOR_COUNT] = {
    "CO", "CO2", "O2", "H2S", "SO2", "NH3"
};

/** @brief 创建统一样式的白色卡片。 */
static lv_obj_t *setting_create_card(lv_obj_t *parent, int32_t height)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_height(card, height);
    lv_obj_set_scrollable(card, false);
    lv_obj_set_style_radius(card, 8, LV_PART_MAIN);
    /* 设置页面不显示卡片外边框，避免小屏幕中边框挤压开关区域。 */
    lv_obj_set_style_border_width(card, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(card, 6, LV_PART_MAIN);
    return card;
}

/** @brief 创建年月日和时分数字显示区域。 */
static void setting_create_clock_card(lv_obj_t *parent)
{
    static int32_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static int32_t row_dsc[] = {22, 36, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *card = setting_create_card(parent, 72);
    lv_obj_set_grid_dsc_array(card, col_dsc, row_dsc);

    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "Date / Time");
    lv_obj_set_grid_cell(title,
                         LV_GRID_ALIGN_CENTER, 0, 2,
                         LV_GRID_ALIGN_CENTER, 0, 1);

    /* 暂时使用数字占位；移植后由STM32 RTC写入年月日。 */
    g_setting_ui.date_label = lv_label_create(card);
    lv_label_set_text(g_setting_ui.date_label, "2026-01-01");
    lv_obj_set_style_text_font(g_setting_ui.date_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_grid_cell(g_setting_ui.date_label,
                         LV_GRID_ALIGN_CENTER, 0, 1,
                         LV_GRID_ALIGN_CENTER, 1, 1);

    /* 暂时使用数字占位；移植后由STM32 RTC写入时和分。 */
    g_setting_ui.time_label = lv_label_create(card);
    lv_label_set_text(g_setting_ui.time_label, "00:00");
    lv_obj_set_style_text_font(g_setting_ui.time_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_grid_cell(g_setting_ui.time_label,
                         LV_GRID_ALIGN_CENTER, 1, 1,
                         LV_GRID_ALIGN_CENTER, 1, 1);
}

/** @brief 创建六路气体传感器连接状态LED。 */
static void setting_create_sensor_card(lv_obj_t *parent)
{
    static int32_t col_dsc[] = {
        LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST
    };
    static int32_t row_dsc[] = {24, 38, 38, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *card = setting_create_card(parent, 118);
    lv_obj_set_grid_dsc_array(card, col_dsc, row_dsc);
    lv_obj_set_style_pad_row(card, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_column(card, 2, LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "Sensor Status  Green: OK  Red: Error");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_12, LV_PART_MAIN);
    lv_obj_set_grid_cell(title,
                         LV_GRID_ALIGN_CENTER, 0, 3,
                         LV_GRID_ALIGN_CENTER, 0, 1);

    for(uint32_t i = 0; i < SETTING_SENSOR_COUNT; i++) {
        lv_obj_t *item = lv_obj_create(card);
        lv_obj_set_scrollable(item, false);
        lv_obj_set_style_border_width(item, 0, LV_PART_MAIN);
        lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_pad_all(item, 2, LV_PART_MAIN);
        lv_obj_set_flex_flow(item, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(item,
                              LV_FLEX_ALIGN_CENTER,
                              LV_FLEX_ALIGN_CENTER,
                              LV_FLEX_ALIGN_CENTER);
        lv_obj_set_style_pad_column(item, 6, LV_PART_MAIN);
        lv_obj_set_grid_cell(item,
                             LV_GRID_ALIGN_STRETCH, i % 3, 1,
                             LV_GRID_ALIGN_STRETCH, 1 + i / 3, 1);

        g_setting_ui.sensor_led[i] = lv_led_create(item);
        lv_obj_set_size(g_setting_ui.sensor_led[i], 14, 14);
        /* 尚未接入传感器检测接口，因此默认使用红色错误状态。 */
        lv_led_set_color(g_setting_ui.sensor_led[i], lv_palette_main(LV_PALETTE_RED));
        lv_led_set_brightness(g_setting_ui.sensor_led[i], 255);

        g_setting_ui.sensor_label[i] = lv_label_create(item);
        lv_label_set_text(g_setting_ui.sensor_label[i], sensor_names[i]);
    }
}

/** @brief 创建当前温度和湿度显示区域。 */
static void setting_create_environment_card(lv_obj_t *parent)
{
    static int32_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static int32_t row_dsc[] = {22, 34, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *card = setting_create_card(parent, 70);
    lv_obj_set_grid_dsc_array(card, col_dsc, row_dsc);

    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "Temperature / Humidity");
    lv_obj_set_grid_cell(title,
                         LV_GRID_ALIGN_CENTER, 0, 2,
                         LV_GRID_ALIGN_CENTER, 0, 1);

    g_setting_ui.temperature_label = lv_label_create(card);
    lv_label_set_text(g_setting_ui.temperature_label, "Temp: --.- C");
    lv_obj_set_grid_cell(g_setting_ui.temperature_label,
                         LV_GRID_ALIGN_CENTER, 0, 1,
                         LV_GRID_ALIGN_CENTER, 1, 1);

    g_setting_ui.humidity_label = lv_label_create(card);
    lv_label_set_text(g_setting_ui.humidity_label, "Humidity: --.- %RH");
    lv_obj_set_grid_cell(g_setting_ui.humidity_label,
                         LV_GRID_ALIGN_CENTER, 1, 1,
                         LV_GRID_ALIGN_CENTER, 1, 1);
}

/** @brief 创建风扇电机和开窗舵机的两个静态开关。 */
static void setting_create_control_card(lv_obj_t *parent)
{
    static int32_t col_dsc[] = {LV_GRID_FR(1), 58, 54, LV_GRID_TEMPLATE_LAST};
    static int32_t row_dsc[] = {22, 34, 34, LV_GRID_TEMPLATE_LAST};

    lv_obj_t *card = setting_create_card(parent, 106);
    lv_obj_set_grid_dsc_array(card, col_dsc, row_dsc);
    /* 覆盖主题默认Grid间距，防止最后一行舵机开关超出卡片被裁剪。 */
    lv_obj_set_style_pad_row(card, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_column(card, 4, LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(card);
    lv_label_set_text(title, "Device Control");
    lv_obj_set_grid_cell(title,
                         LV_GRID_ALIGN_CENTER, 0, 3,
                         LV_GRID_ALIGN_CENTER, 0, 1);

    lv_obj_t *fan_label = lv_label_create(card);
    lv_label_set_text(fan_label, "Fan Motor");
    lv_obj_set_grid_cell(fan_label,
                         LV_GRID_ALIGN_START, 0, 1,
                         LV_GRID_ALIGN_CENTER, 1, 1);

    g_setting_ui.fan_switch = lv_switch_create(card);
    lv_obj_set_grid_cell(g_setting_ui.fan_switch,
                         LV_GRID_ALIGN_CENTER, 1, 1,
                         LV_GRID_ALIGN_CENTER, 1, 1);

    g_setting_ui.fan_state_label = lv_label_create(card);
    lv_label_set_text(g_setting_ui.fan_state_label, "OFF");
    lv_obj_set_grid_cell(g_setting_ui.fan_state_label,
                         LV_GRID_ALIGN_CENTER, 2, 1,
                         LV_GRID_ALIGN_CENTER, 1, 1);

    lv_obj_t *window_label = lv_label_create(card);
    lv_label_set_text(window_label, "Window Servo");
    lv_obj_set_grid_cell(window_label,
                         LV_GRID_ALIGN_START, 0, 1,
                         LV_GRID_ALIGN_CENTER, 2, 1);

    g_setting_ui.window_switch = lv_switch_create(card);
    lv_obj_set_grid_cell(g_setting_ui.window_switch,
                         LV_GRID_ALIGN_CENTER, 1, 1,
                         LV_GRID_ALIGN_CENTER, 2, 1);

    g_setting_ui.window_state_label = lv_label_create(card);
    lv_label_set_text(g_setting_ui.window_state_label, "OFF");
    lv_obj_set_grid_cell(g_setting_ui.window_state_label,
                         LV_GRID_ALIGN_CENTER, 2, 1,
                         LV_GRID_ALIGN_CENTER, 2, 1);
}

void setting_create(lv_obj_t *parent)
{
    if(parent == NULL) {
        return;
    }

    g_setting_ui = (setting_ui_st){0};

    /* 根容器固定为页面大小，内部卡片超过高度后由该容器负责纵向滚动。 */
    g_setting_ui.root = lv_obj_create(parent);
    lv_obj_set_size(g_setting_ui.root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_border_width(g_setting_ui.root, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(g_setting_ui.root, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(g_setting_ui.root, 6, LV_PART_MAIN);
    /*
     * 为最下面的舵机开关保留额外滚动空间。
     * Windows模拟器底部启用了LVGL性能/内存监控层，如果没有这段留白，
     * 最后一行即使滚动到底也会被灰色监控信息覆盖。
     */
    lv_obj_set_style_pad_bottom(g_setting_ui.root, 48, LV_PART_MAIN);
    lv_obj_set_style_pad_row(g_setting_ui.root, 8, LV_PART_MAIN);
    lv_obj_set_flex_flow(g_setting_ui.root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(g_setting_ui.root,
                          LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scroll_dir(g_setting_ui.root, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(g_setting_ui.root, LV_SCROLLBAR_MODE_AUTO);

    setting_create_clock_card(g_setting_ui.root);
    setting_create_sensor_card(g_setting_ui.root);
    setting_create_environment_card(g_setting_ui.root);
    setting_create_control_card(g_setting_ui.root);
}
