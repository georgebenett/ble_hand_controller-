#include "menu.h"
#include "esp_log.h"
#include "lcd.h"
#include "nvs_flash.h"
#include "nvs.h"

#define TAG "MENU"

static menu_state_t current_state = MENU_STATE_HIDDEN;
static lv_obj_t* menu_container = NULL;
static lv_obj_t* menu_list = NULL;
static int selected_index = 0;

static skate_config_t skate_config = {
    .motor_pulley = 15,   // Default values
    .wheel_pulley = 36,
    .wheel_size = 83
};

static int motor_pulley_value = 15;  // Default value

static void highlight_selected_item(void) {
    if (menu_list == NULL) {
        return;
    }

    uint32_t item_count = lv_obj_get_child_cnt(menu_list);

    for (uint32_t i = 0; i < item_count; i++) {
        lv_obj_t* item = lv_obj_get_child(menu_list, i);
        if (item == NULL) {
            continue;
        }

        if (i == selected_index) {
            lv_obj_set_style_text_font(item, &lv_font_montserrat_30, 0);  // Bigger font for selected
            lv_obj_set_style_text_color(item, lv_color_white(), 0);  // White for selected
        } else {
            lv_obj_set_style_text_font(item, &lv_font_montserrat_28, 0);  // Normal font for unselected
            lv_obj_set_style_text_color(item, lv_color_white(), 0);  // White for unselected too
        }
    }
}

static void create_main_menu(void) {
    menu_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(menu_container, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_style_bg_color(menu_container, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(menu_container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(menu_container, 0, 0);

    menu_list = lv_list_create(menu_container);
    lv_obj_set_size(menu_list, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_style_pad_all(menu_list, 0, 0);
    lv_obj_align(menu_list, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_bg_color(menu_list, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(menu_list, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(menu_list, 0, 0);

    // Create menu items with larger text
    lv_obj_t* item1 = lv_list_add_text(menu_list, "Throttle Config");
    lv_obj_t* item2 = lv_list_add_text(menu_list, "Skate Config");

    // Style each item
    lv_obj_set_style_bg_color(item1, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(item1, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(item2, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(item2, LV_OPA_COVER, 0);

    // Make text larger and set initial color
    lv_obj_set_style_text_font(item1, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_font(item2, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(item1, lv_color_white(), 0);
    lv_obj_set_style_text_color(item2, lv_color_white(), 0);

    selected_index = 0;  // Reset selection
    highlight_selected_item();
}

static void create_skate_menu(void) {
    menu_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(menu_container, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_style_bg_color(menu_container, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(menu_container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(menu_container, 0, 0);

    menu_list = lv_list_create(menu_container);
    lv_obj_set_size(menu_list, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_style_pad_all(menu_list, 0, 0);
    lv_obj_align(menu_list, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_bg_color(menu_list, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(menu_container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(menu_list, 0, 0);

    // Create menu items with larger text
    lv_obj_t* item1 = lv_list_add_text(menu_list, "Motor Pulley");
    lv_obj_t* item2 = lv_list_add_text(menu_list, "Wheel Pulley");
    lv_obj_t* item3 = lv_list_add_text(menu_list, "Wheel Size");

    // Style each item
    lv_obj_set_style_bg_color(item1, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(item1, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(item2, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(item2, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(item3, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(item3, LV_OPA_COVER, 0);

    // Make text larger and set initial color
    lv_obj_set_style_text_font(item1, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_font(item2, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_font(item3, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(item1, lv_color_white(), 0);
    lv_obj_set_style_text_color(item2, lv_color_white(), 0);
    lv_obj_set_style_text_color(item3, lv_color_white(), 0);

    selected_index = 0;  // Reset selection
    highlight_selected_item();
}

static void create_motor_pulley_menu(void) {
    menu_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(menu_container, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_style_bg_color(menu_container, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(menu_container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(menu_container, 0, 0);

    menu_list = lv_list_create(menu_container);
    lv_obj_set_size(menu_list, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_style_pad_all(menu_list, 0, 0);
    lv_obj_align(menu_list, LV_ALIGN_TOP_MID, 80, 50);
    lv_obj_set_style_bg_color(menu_list, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(menu_list, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(menu_list, 0, 0);

    // Create value display
    char value_str[8];
    snprintf(value_str, sizeof(value_str), "%d", motor_pulley_value);
    lv_obj_t* value_label = lv_list_add_text(menu_list, value_str);
    
    // Style the value display
    lv_obj_set_style_text_font(value_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(value_label, lv_color_white(), 0);
    lv_obj_set_style_bg_color(value_label, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(value_label, LV_OPA_COVER, 0);
}

void menu_init(void) {
    // Initial setup if needed
}

void menu_show(void) {
    if (current_state == MENU_STATE_HIDDEN) {
        create_main_menu();
        current_state = MENU_STATE_MAIN;
    }
}

void menu_hide(void) {
    if (menu_container != NULL) {
        lv_obj_del(menu_container);
        menu_container = NULL;
        menu_list = NULL;
        current_state = MENU_STATE_HIDDEN;
    }
}

menu_state_t menu_get_state(void) {
    return current_state;
}

void menu_select_item(void) {
    if (current_state == MENU_STATE_MAIN) {
        if (selected_index == 1) {  // Skate Config selected
            menu_hide();
            create_skate_menu();
            current_state = MENU_STATE_SKATE;
        }
    } else if (current_state == MENU_STATE_SKATE) {
        if (selected_index == 0) {  // Motor Pulley selected
            menu_hide();
            create_motor_pulley_menu();
            current_state = MENU_STATE_MOTOR_PULLEY;
        }
    }
}

void menu_back(void) {
    if (current_state == MENU_STATE_MOTOR_PULLEY) {
        menu_save_skate_config();  // Save value before exiting
        menu_hide();
        create_skate_menu();
        current_state = MENU_STATE_SKATE;
    } else if (current_state != MENU_STATE_MAIN) {
        menu_hide();
        create_main_menu();
        current_state = MENU_STATE_MAIN;
    } else {
        menu_hide();
    }
}

void menu_handle_navigation(float adc_value) {
    if (current_state == MENU_STATE_HIDDEN) {
        return;
    }

    static uint32_t last_change_time = 0;
    uint32_t current_time = xTaskGetTickCount();

    if ((current_time - last_change_time) * portTICK_PERIOD_MS < 200) {
        return;
    }

    if (current_state == MENU_STATE_MOTOR_PULLEY) {
        // Map ADC value (0-255) to pulley range (10-30)
        int new_value = 10 + (adc_value * 20) / 255;
        if (new_value != motor_pulley_value) {
            motor_pulley_value = new_value;
            char value_str[8];
            snprintf(value_str, sizeof(value_str), "%d", motor_pulley_value);
            lv_label_set_text(lv_obj_get_child(menu_list, 0), value_str);
            last_change_time = current_time;
        }
        return;
    }

    uint32_t item_count = lv_obj_get_child_cnt(menu_list);

    if (adc_value > 200) {  // Move down
        if (selected_index < item_count - 1) {
            selected_index++;
            highlight_selected_item();
            last_change_time = current_time;
        }
    } else if (adc_value < 50) {  // Move up
        if (selected_index > 0) {
            selected_index--;
            highlight_selected_item();
            last_change_time = current_time;
        }
    }
}

void menu_set_selected_index(int index) {
    selected_index = index;
    highlight_selected_item();
}

int menu_get_selected_index(void) {
    return selected_index;
}

void menu_save_skate_config(void) {
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvs_handle));

    ESP_ERROR_CHECK(nvs_set_u8(nvs_handle, "motor_pulley", skate_config.motor_pulley));
    ESP_ERROR_CHECK(nvs_set_u8(nvs_handle, "wheel_pulley", skate_config.wheel_pulley));
    ESP_ERROR_CHECK(nvs_set_u8(nvs_handle, "wheel_size", skate_config.wheel_size));

    ESP_ERROR_CHECK(nvs_commit(nvs_handle));
    nvs_close(nvs_handle);
}

skate_config_t menu_get_skate_config(void) {
    nvs_handle_t nvs_handle;
    ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvs_handle));

    uint8_t value;
    if (nvs_get_u8(nvs_handle, "motor_pulley", &value) == ESP_OK) {
        skate_config.motor_pulley = value;
    }
    if (nvs_get_u8(nvs_handle, "wheel_pulley", &value) == ESP_OK) {
        skate_config.wheel_pulley = value;
    }
    if (nvs_get_u8(nvs_handle, "wheel_size", &value) == ESP_OK) {
        skate_config.wheel_size = value;
    }

    nvs_close(nvs_handle);
    return skate_config;
}
