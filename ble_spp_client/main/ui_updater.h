#ifndef UI_UPDATER_H
#define UI_UPDATER_H

#include "ui/ui.h"

extern const lv_img_dsc_t ui_img_no_connection_png;
extern const lv_img_dsc_t ui_img_33_connection_png;
extern const lv_img_dsc_t ui_img_66_connection_png;
extern const lv_img_dsc_t ui_img_full_connection_png;

void ui_updater_init(void);
void ui_update_speed(int32_t value);
void ui_update_battery_voltage(float voltage);
void ui_update_motor_current(float current);
void ui_update_battery_current(float current);
void ui_update_consumption(float consumption);
void ui_update_connection_quality(int rssi);
void ui_update_connection_icon(void);

#endif // UI_UPDATER_H 