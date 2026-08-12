#ifndef __LV_ARC_DEMO_H
#define __LV_ARC_DEMO_H

/**
 * @brief 创建由LVGL圆弧例程 适配而来的圆弧验证界面。
 *
 * 本函数只负责创建界面，不包含 LVGL 初始化和循环调度。
 * 调用前必须依次完成 lv_init()、显示驱动注册以及 LVGL 时基注册。
 */
void lv_arc_demo_create(void);

#endif
