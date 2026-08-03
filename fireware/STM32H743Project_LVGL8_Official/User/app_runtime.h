#ifndef __APP_RUNTIME_H
#define __APP_RUNTIME_H

#include "./UI/page.h"

extern volatile uint8_t alarm_flag;

#define ALARM_FANG_DANGLE 1
#define ALARM_FANG_WORN 2
#define ALARM_FANG_SAFE 3

/** 在main()中调用：初始化RTC，并在RTC内容无效时写入初始时间。 */
void app_runtime_hardware_init(void);

/** 在UI创建完成后调用：设置连接标志并立即刷新一次实时数据。 */
void app_runtime_ui_start(void);

/** 在LVGL任务循环中调用：每秒刷新RTC、六个仪表盘和温湿度。 */
void app_runtime_ui_process(void);

/** 在LVGL任务循环中调用：分析六路气体数据，判断是否告警。*/
void app_runtime_analyze(void);

#endif
