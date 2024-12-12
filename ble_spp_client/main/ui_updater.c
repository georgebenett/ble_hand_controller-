#include "ui_updater.h"
#include "esp_log.h"
#include "adc.h"

#define TAG "UI_UPDATER"

static lv_obj_t* get_current_screen(void) {
    return lv_scr_act();
}

void ui_updater_init(void) {
    // Any initialization code if needed
}

void ui_update_speed(int32_t value) {
    if (ui_Label1 == NULL) return;

    // Only update if home screen is active
    if (get_current_screen() == ui_home_screen) {
        lv_label_set_text_fmt(ui_Label1, "%ld", value);
    }
}

void ui_update_battery_voltage(float voltage) {
    if (ui_vesc_voltage == NULL) return;

    // Only update if detailed screen is active
    if (get_current_screen() == ui_detailed_home) {
        lv_label_set_text_fmt(ui_vesc_voltage, "%.1fv", voltage);
    }
}

void ui_update_motor_current(float current) {
    if (ui_vesc_motor_current == NULL) return;

    // Only update if detailed screen is active
    if (get_current_screen() == ui_detailed_home) {
        lv_label_set_text_fmt(ui_vesc_motor_current, "%.1fa", current);
    }
}

void ui_update_battery_current(float current) {
    if (ui_battery_current == NULL) return;

    // Only update if detailed screen is active
    if (get_current_screen() == ui_detailed_home) {
        lv_label_set_text_fmt(ui_battery_current, "%.1fa", current);
    }
}

void ui_update_consumption(float consumption) {
    if (ui_vesc_consumption == NULL) return;

    // Only update if detailed screen is active
    if (get_current_screen() == ui_detailed_home) {
        lv_label_set_text_fmt(ui_vesc_consumption, "%.1fwh", consumption);
    }
}