#ifndef UI_UPDATER_H
#define UI_UPDATER_H

#include <stdint.h>
#include "ui/ui.h"

// Function to initialize the UI updater
void ui_updater_init(void);

// Functions to update specific UI elements
void ui_update_speed(int32_t value);
void ui_update_battery_voltage(float voltage);
void ui_update_motor_current(float current);
void ui_update_battery_current(float current);
void ui_update_consumption(float consumption);

#endif // UI_UPDATER_H 