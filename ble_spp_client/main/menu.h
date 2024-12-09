#ifndef MENU_H
#define MENU_H

#include <stdbool.h>
#include "lvgl.h"

// Menu states
typedef enum {
    MENU_STATE_HIDDEN,
    MENU_STATE_MAIN,
    MENU_STATE_THROTTLE,
    MENU_STATE_SKATE
} menu_state_t;

// Initialize menu system
void menu_init(void);

// Show/hide menu
void menu_show(void);
void menu_hide(void);

// Get current menu state
menu_state_t menu_get_state(void);

// Menu navigation
void menu_select_item(void);
void menu_back(void);

// Add these declarations
void menu_handle_navigation(float adc_value);
void menu_set_selected_index(int index);
int menu_get_selected_index(void);

#endif // MENU_H