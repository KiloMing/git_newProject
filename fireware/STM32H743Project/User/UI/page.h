#ifndef PAGE_H
#define PAGE_H

#include "lvgl.h"
#include "testvalue.h"
#include "chart.h"
#include "setting.h"
#include "app_interface.h"
extern testvalue_st g_test_array[6];
extern chart_st g_chart_array[6];


/**
 * @brief 创建标签页
 *
 * Call this function after lv_init() and after the display/input
 * drivers have been initialized.
 */
void page_create(lv_obj_t *parent);


#endif /* PAGE_H */
