#include "page.h"

/// 网格布局描述符
static int32_t col_dsc[] = {
    260,
    260,
    260,
    LV_GRID_TEMPLATE_LAST
};

static int32_t row_dsc[] = {
    215,
    215,
    LV_GRID_TEMPLATE_LAST
};

testvalue_st g_test_array[6] = {0};

/* 历史数据页的两个图表需要在page_create()返回后继续存在，因此使用静态存储。 */
chart_st g_chart_array[6] = {0};
/* 00:00~23:00的模拟历史数据，用于第一阶段验证阈值线和分段颜色。 */
static float chart_data_1[24] = {
    3, 4, 5, 6, 8, 10, 12, 15, 18, 25, 32, 36,
    40, 38, 30, 22, 16, 12, 9, 7, 6, 5, 4, 3
};
static float chart_data_2[24] = {
    450, 480, 520, 600, 750, 900, 1100, 1500, 2200, 3200, 4200, 5100,
    5300, 4800, 3800, 2500, 1600, 1100, 900, 750, 650, 550, 500, 460
};
static float chart_data_3[24] = {
    20.9f, 20.8f, 20.7f, 20.6f, 20.4f, 20.1f, 19.7f, 19.3f,
    18.8f, 19.6f, 20.4f, 20.9f, 21.2f, 21.6f, 22.2f, 23.0f,
    23.7f, 24.1f, 23.2f, 22.0f, 21.4f, 21.0f, 20.9f, 20.8f
};
static float chart_data_4[24] = {
    0.1f, 0.2f, 0.3f, 0.5f, 0.8f, 1.0f, 1.5f, 2.5f,
    4.0f, 7.0f, 9.5f, 10.0f, 11.0f, 13.0f, 8.0f, 5.0f,
    2.0f, 1.2f, 0.9f, 0.7f, 0.5f, 0.3f, 0.2f, 0.1f
};
static float chart_data_5[24] = {
    0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.7f, 1.0f, 1.5f,
    1.9f, 2.0f, 2.5f, 3.5f, 5.0f, 4.0f, 2.2f, 1.8f,
    1.2f, 0.8f, 0.5f, 0.4f, 0.3f, 0.2f, 0.2f, 0.1f
};
static float chart_data_6[24] = {
    1, 2, 3, 4, 5, 7, 10, 15, 20, 24, 25, 30,
    40, 35, 28, 22, 18, 12, 8, 5, 4, 3, 2, 1
};

void page_create(lv_obj_t *parent)
{
    /* 创建一个标签页 */
    lv_obj_t *tabview = lv_tabview_create(parent);
    lv_obj_set_size(tabview, LV_PCT(100), LV_PCT(100));
    lv_tabview_set_tab_bar_size(tabview, 30);
    /* 添加标签页 */
    lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "CurrentValues");
    lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "PastValues");
    lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Settings");
    lv_obj_set_style_border_width(tab1, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(tab1, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(tab2, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(tab2, 0, LV_PART_MAIN);
    lv_obj_set_style_border_width(tab3, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(tab3, 0, LV_PART_MAIN);
/***************************PAGE 1******************************/
    /* tab1中创建6个网格用于存放6个检测仪表 */
    lv_obj_t *grid = lv_obj_create(tab1);
    lv_obj_set_style_border_width(grid, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(grid, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(grid, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_pad_all(grid, 0, LV_PART_MAIN);
    /* NT35510横屏为800x480，减去30像素标签栏后，内容区固定使用780x430。 */
    lv_obj_set_size(grid, 780, 430);

    /*
     * 当前先针对NT35510的800x480屏验证纯显示。
     * 暂时不启用Grid自动布局，避免布局计算异常导致六个子容器尺寸为0或移出屏幕。
     * col_dsc/row_dsc仍然保留，确认显示正常后可以重新启用。
     */
    //将grid放置在tab1中
    lv_obj_center(grid);
    //在grid中创建6个检测仪表
    testvalue_st *co_value = testvalue_create(&g_test_array[0], grid, "CO", "ppm", 0.0f, 0.0f);
    testvalue_st *co2_value = testvalue_create(&g_test_array[1], grid, "CO2", "ppm", 0.0f, 400.0f);
    testvalue_st *o2_value = testvalue_create(&g_test_array[2], grid, "O2", "%vol", 0.0f, 20.9f);
    testvalue_st *h2s_value = testvalue_create(&g_test_array[3], grid, "H2S", "ppm", 0.0f, 0.0f);
    testvalue_st *so2_value = testvalue_create(&g_test_array[4], grid, "SO2", "ppm", 0.0f, 0.0f);
    testvalue_st *nh3_value = testvalue_create(&g_test_array[5], grid, "NH3", "ppm", 0.0f, 0.0f);

    /* 按气体安全表配置阈值。配置完成后会自动计算初始危险百分比和颜色。 */
    testvalue_set_high_thresholds(co_value, 10.0f, 35.0f, 100.0f, true);
    testvalue_set_high_thresholds(co2_value, 1000.0f, 5000.0f, 5000.0f, true);
    testvalue_set_band_thresholds(o2_value, 15.0f, 19.5f, 20.5f, 21.5f, 23.5f, 25.0f);
    testvalue_set_high_thresholds(h2s_value, 1.0f, 10.0f, 20.0f, false);
    testvalue_set_high_thresholds(so2_value, 0.5f, 2.0f, 10.0f, true);
    testvalue_set_high_thresholds(nh3_value, 5.0f, 25.0f, 100.0f, true);
    /*
     * 使用固定坐标验证第一页：三列两行，每个容器250x205。
     * 这种布局不依赖Grid计算，能够保证所有仪表盘都位于780x430可见区域内。
     */
    lv_obj_set_size(co_value->container, 250, 205);
    lv_obj_set_pos(co_value->container, 5, 5);
    lv_obj_set_size(co2_value->container, 250, 205);
    lv_obj_set_pos(co2_value->container, 265, 5);
    lv_obj_set_size(o2_value->container, 250, 205);
    lv_obj_set_pos(o2_value->container, 525, 5);
    lv_obj_set_size(h2s_value->container, 250, 205);
    lv_obj_set_pos(h2s_value->container, 5, 220);
    lv_obj_set_size(so2_value->container, 250, 205);
    lv_obj_set_pos(so2_value->container, 265, 220);
    lv_obj_set_size(nh3_value->container, 250, 205);
    lv_obj_set_pos(nh3_value->container, 525, 220);

    /* 立即计算第一页布局，确保第一次刷新前六个容器已经得到有效坐标和尺寸。 */
    lv_obj_update_layout(grid);


/*************************PAGE 2**************************************/
    chart_st *chart1 = create_simple_chart(&g_chart_array[0], tab2, "CO History", chart_data_1, &g_test_array[0]);
    //设置图表大小
    lv_obj_set_size(chart1->chart_container, 430, 205);
    chart_st *chart2 = create_simple_chart(&g_chart_array[1], tab2, "CO2 History", chart_data_2, &g_test_array[1]);
    lv_obj_set_size(chart2->chart_container, 430, 205);
    chart_st *chart3 = create_simple_chart(&g_chart_array[2], tab2, "O2 History", chart_data_3, &g_test_array[2]);
    lv_obj_set_size(chart3->chart_container, 430, 205);
    chart_st *chart4 = create_simple_chart(&g_chart_array[3], tab2, "H2S History", chart_data_4, &g_test_array[3]);
    lv_obj_set_size(chart4->chart_container, 430, 205);
    chart_st *chart5 = create_simple_chart(&g_chart_array[4], tab2, "SO2 History", chart_data_5, &g_test_array[4]);
    lv_obj_set_size(chart5->chart_container, 430, 205);
    chart_st *chart6 = create_simple_chart(&g_chart_array[5], tab2, "NH3 History", chart_data_6, &g_test_array[5]);
    lv_obj_set_size(chart6->chart_container, 430, 205);
    //使用flex布局图表垂直排列
    lv_obj_set_flex_flow(tab2, LV_FLEX_FLOW_COLUMN);
    //设置对齐方式
    lv_obj_set_flex_align(tab2,
                      LV_FLEX_ALIGN_START,
                      LV_FLEX_ALIGN_CENTER,
                      LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scroll_dir(tab2, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(tab2, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_pad_row(tab2, 8, LV_PART_MAIN);


/*************************PAGE 3**************************************/
    /* 设置页组件统一放在setting.c中，page.c只负责传入父对象。 */
    setting_create(tab3);
    /* 所有控件创建完成后，绑定统一接口中的开关事件并同步初始标志位。 */
    app_interface_init();
}
