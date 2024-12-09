#include "menu.h"
#include "esp_log.h"
#include "lcd.h"

#define TAG "MENU"

static menu_state_t current_state = MENU_STATE_HIDDEN;
static lv_obj_t* menu_container = NULL;
static lv_obj_t* menu_list = NULL;
static int selected_index = 0;

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
            lv_obj_set_style_bg_color(item, lv_color_make(0, 255, 0), 0);  // Green background
            lv_obj_set_style_bg_opa(item, LV_OPA_50, 0);
        } else {
            lv_obj_set_style_bg_opa(item, LV_OPA_0, 0);
        }
    }
}

static void create_main_menu(void) {
    menu_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(menu_container, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_style_bg_color(menu_container, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(menu_container, LV_OPA_COVER, 0);

    menu_list = lv_list_create(menu_container);
    lv_obj_set_size(menu_list, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_style_pad_all(menu_list, 0, 0);  // Remove padding
    lv_obj_align(menu_list, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_bg_color(menu_list, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(menu_list, LV_OPA_COVER, 0);

    // Create menu items with larger text
    lv_obj_t* item1 = lv_list_add_text(menu_list, "Throttle Config");
    lv_obj_t* item2 = lv_list_add_text(menu_list, "Skate Config");

    // Make text larger and white
    lv_obj_set_style_text_font(item1, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_font(item2, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_color(item1, lv_color_white(), 0);
    lv_obj_set_style_text_color(item2, lv_color_white(), 0);

    selected_index = 0;  // Reset selection
    highlight_selected_item();  // Highlight first item
}

static void create_skate_menu(void) {
    menu_container = lv_obj_create(lv_scr_act());
    lv_obj_set_size(menu_container, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_style_bg_color(menu_container, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(menu_container, LV_OPA_COVER, 0);

    menu_list = lv_list_create(menu_container);
    lv_obj_set_size(menu_list, LV_HOR_RES_MAX, LV_VER_RES_MAX);
    lv_obj_set_style_pad_all(menu_list, 0, 0);
    lv_obj_align(menu_list, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_bg_color(menu_list, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(menu_list, LV_OPA_COVER, 0);

    // Create menu items with larger text
    lv_obj_t* item1 = lv_list_add_text(menu_list, "Motor Pulley");
    lv_obj_t* item2 = lv_list_add_text(menu_list, "Wheel Pulley");
    lv_obj_t* item3 = lv_list_add_text(menu_list, "Wheel Size");

    // Make text larger and white
    lv_obj_set_style_text_font(item1, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_font(item2, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_font(item3, &lv_font_montserrat_30, 0);
    lv_obj_set_style_text_color(item1, lv_color_white(), 0);
    lv_obj_set_style_text_color(item2, lv_color_white(), 0);
    lv_obj_set_style_text_color(item3, lv_color_white(), 0);

    selected_index = 0;  // Reset selection
    highlight_selected_item();
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
    }
}

void menu_back(void) {
    if (current_state != MENU_STATE_MAIN) {
        // Return to main menu
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