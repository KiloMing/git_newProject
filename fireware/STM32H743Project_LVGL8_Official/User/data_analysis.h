#ifndef __DATA_ANALYSIS_H
#define __DATA_ANALYSIS_H

#include "app_runtime.h"
#include "lvgl_demo.h"
#include "./BSP/LED/led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "./BSP/LORA/lora.h"
#include "lvgl.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "LVGL/GUI_APP/lv_mainstart.h"
#include "app_runtime.h"

/**
 * @brief       创建数据分析任务和告警任务
 * @param       无
 * @retval      无
 */
void data_analysis_creat(void);


#endif
