/**
 ****************************************************************************************************
 * @file        lv_port_indev.c
 * @brief       正点原子触摸驱动到 LVGL 9 的输入设备移植层
 *
 * @note
 * - 底层直接使用官方 touch.c 中的 tp_dev.init() 和 tp_dev.scan()。
 * - NT35510（LCD ID 0x5510）由官方驱动选择 GT9xxx 电容触摸控制器。
 * - LCD必须先设置好横竖屏方向，之后才能初始化触摸，否则坐标方向会不一致。
 ****************************************************************************************************
 */

#include "lv_port_indev.h"
#include "./BSP/TOUCH/touch.h"
#include "./BSP/LCD/lcd.h"

static lv_indev_t *s_touch_indev;

/**
 * @brief LVGL周期调用的触摸读取回调。
 *
 * tp_dev.scan(0) 返回已经由正点原子驱动转换后的屏幕坐标，因此这里不再交换X/Y。
 */
static void lv_port_touch_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    static int32_t last_x;
    static int32_t last_y;

    (void)indev;
    tp_dev.scan(0);

    if((tp_dev.sta & TP_PRES_DOWN) != 0U) {
        last_x = (int32_t)tp_dev.x[0];
        last_y = (int32_t)tp_dev.y[0];

        /* 异常坐标不允许传入LVGL，防止对象命中测试访问屏幕范围之外。 */
        if(last_x < 0) last_x = 0;
        if(last_y < 0) last_y = 0;
        if(last_x >= (int32_t)lcddev.width) last_x = (int32_t)lcddev.width - 1;
        if(last_y >= (int32_t)lcddev.height) last_y = (int32_t)lcddev.height - 1;

        data->state = LV_INDEV_STATE_PRESSED;
    }
    else {
        data->state = LV_INDEV_STATE_RELEASED;
    }

    data->point.x = last_x;
    data->point.y = last_y;
}

void lv_port_indev_init(void)
{
    /* 官方要求：LCD初始化和lcd_display_dir()必须在触摸初始化之前完成。 */
    tp_dev.init();

    s_touch_indev = lv_indev_create();
    if(s_touch_indev == NULL) {
        return;
    }

    lv_indev_set_type(s_touch_indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(s_touch_indev, lv_port_touch_read);
    lv_indev_set_display(s_touch_indev, lv_display_get_default());
}

lv_indev_t *lv_port_indev_get_touchpad(void)
{
    return s_touch_indev;
}
