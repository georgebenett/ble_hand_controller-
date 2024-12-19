#include "ui_updater.h"
#include "esp_log.h"
#include "adc.h"
#include "ble_spp_client.h"
#include "ui/ui.h"



#define TAG "UI_UPDATER"

static uint8_t connection_quality = 0;

static lv_obj_t* get_current_screen(void) {
    return lv_scr_act();
}

void ui_update_connection_quality(int rssi) {
    // If RSSI is 0 or positive, consider it as disconnected
    if (rssi >= 0) {
        connection_quality = 0;
    } else {
        // Normalize RSSI to percentage
        connection_quality = ((rssi + 100) * 100) / 70;

        // Clamp percentage between 0 and 100
        if (connection_quality > 100) connection_quality = 100;

    }
    // Update the UI
    ui_update_connection_icon();
}

void ui_update_connection_icon(void) {
    if (ui_no_connection_icon == NULL) return;

    // Only update if home screen is active
    if (get_current_screen() == ui_home_screen) {
        if (!is_connect) {
            lv_img_set_src(ui_no_connection_icon, &ui_img_no_connection_png);
            return;
        }

        if (connection_quality < 15) {
            lv_img_set_src(ui_no_connection_icon, &ui_img_33_connection_png);
        }
        else if (connection_quality < 35) {
            lv_img_set_src(ui_no_connection_icon, &ui_img_66_connection_png);
        }
        else {
            lv_img_set_src(ui_no_connection_icon, &ui_img_full_connection_png);
        }
    }
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

int get_connection_quality(void) {
    return connection_quality;
}