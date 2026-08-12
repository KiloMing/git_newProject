/**
 * @file lv_port_disp.c
 * @brief LVGL 9与FMC MCU屏驱动之间的显示移植层。
 *
 * LCD接口为16位RGB565。LVGL只渲染若干行到外部SDRAM中的局部缓冲区，
 * 随后本文件把这些像素连续写入LCD GRAM，从而避免申请完整帧缓冲。
 */

#include "lv_port_disp.h"

#include "lvgl.h"
#include "./BSP/LCD/lcd.h"

/* LCD BSP驱动支持的最大横向分辨率为800像素。 */
#define LV_PORT_DISP_MAX_WIDTH       800U

/* 每次渲染20行：800 * 20 * 2 = 32KB，位于1MB LVGL内存池之后。 */
#define LV_PORT_DISP_BUFFER_LINES    10U
#define LV_PORT_DISP_BUFFER_PIXELS   (LV_PORT_DISP_MAX_WIDTH * LV_PORT_DISP_BUFFER_LINES)
#define LV_PORT_DISP_BUFFER_BYTES    (LV_PORT_DISP_BUFFER_PIXELS * 2U)

/* 与官方例程相同，先用片内静态 RAM 缓冲 10 行 RGB565 像素。 */
static uint16_t s_lvgl_draw_buffer[LV_PORT_DISP_BUFFER_PIXELS];

/**
 * @brief 把LVGL给出的矩形区域写入LCD。
 *
 * @param display LVGL显示设备对象。
 * @param area    本次需要刷新的闭区间坐标，x2/y2均包含在刷新范围内。
 * @param px_map  RGB565像素数组，按从左到右、从上到下的顺序排列。
 */
static void lv_port_disp_flush(lv_display_t *display,
                               const lv_area_t *area,
                               uint8_t *px_map)
{
    /*
     * 直接复用LCD BSP驱动已经验证的彩色块写入函数。
     * 对NT35510（ID 0x5510），该函数会逐行调用lcd_set_cursor()，并使用
     * 0x2A00/0x2B00/0x2C00寄存器写法；不能假设它与SSD1963完全相同，
     * 在只设置一次窗口后就能可靠地跨越多行连续写入。
     */
    lcd_color_fill((uint16_t)area->x1,
                   (uint16_t)area->y1,
                   (uint16_t)area->x2,
                   (uint16_t)area->y2,
                   (uint16_t *)px_map);

    /* 必须在全部像素写完后通知LVGL，否则LVGL可能过早复用绘图缓冲。 */
    lv_display_flush_ready(display);
}

void lv_port_disp_init(void)
{
    lv_display_t *display;
    void *draw_buffer;

    /* 防止未知LCD分辨率超过预留缓冲区一行的容量。 */
    if((lcddev.width == 0U) || (lcddev.height == 0U) ||
       (lcddev.width > LV_PORT_DISP_MAX_WIDTH)) {
        return;
    }

    draw_buffer = (void *)s_lvgl_draw_buffer;
    display = lv_display_create((int32_t)lcddev.width, (int32_t)lcddev.height);
    lv_display_set_color_format(display, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(display, lv_port_disp_flush);
    lv_display_set_buffers(display,
                           draw_buffer,
                           NULL,
                           LV_PORT_DISP_BUFFER_BYTES,
                           LV_DISPLAY_RENDER_MODE_PARTIAL);
}
