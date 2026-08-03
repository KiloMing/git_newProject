#ifndef UI_H
#define UI_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create and load the application UI.
 *
 * Call this function after lv_init() and after the display/input
 * drivers have been initialized.
 */
void ui_init(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_H */
