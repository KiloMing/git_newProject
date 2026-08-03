#ifndef __LV_PORT_INDEV_H
#define __LV_PORT_INDEV_H

#include "lvgl.h"

/** 初始化正点原子触摸驱动并注册为 LVGL 指针输入设备。 */
void lv_port_indev_init(void);

/** 获取已注册的触摸输入设备，未初始化时返回 NULL。 */
lv_indev_t *lv_port_indev_get_touchpad(void);

#endif
