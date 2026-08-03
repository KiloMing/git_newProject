#ifndef LV_PORT_DISP_H
#define LV_PORT_DISP_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 将LVGL显示设备连接到开发板现有的FMC 16位LCD驱动。
 *
 * 调用顺序必须是：lcd_function_init() -> lv_init() -> lv_port_disp_init()。
 * 本函数使用lcddev.width和lcddev.height，因此会自动采用LCD驱动识别出的横屏分辨率。
 */
void lv_port_disp_init(void);

#ifdef __cplusplus
}
#endif

#endif /* LV_PORT_DISP_H */
