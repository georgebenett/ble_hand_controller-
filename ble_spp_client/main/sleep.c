#include "sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lcd.h"
#include "ui/ui.h"
#include "lvgl.h"
#include "esp_sleep.h"

#define TAG "SLEEP"

static TickType_t last_activity_time;
static TickType_t last_reset_time = 0;
#define RESET_DEBOUNCE_TIME_MS 2000

static lv_anim_t arc_anim;
static bool arc_animation_active = false;

static void set_arc_value(void * obj, int32_t v)
{
    lv_arc_set_value(obj, v);
    
    // If we reach 100%, trigger sleep immediately
    if (v >= 100) {
        ESP_LOGI(TAG, "Arc filled - Entering deep sleep mode");
        // Configure wakeup on button press (transition from HIGH to LOW)
        ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(1ULL << MAIN_BUTTON_GPIO,
                                                      ESP_GPIO_WAKEUP_GPIO_LOW));
        vTaskDelay(pdMS_TO_TICKS(2000));
        esp_deep_sleep_start();
    }
}

static void sleep_button_callback(button_event_t event, void* user_data) {
    switch(event) {
        case BUTTON_EVENT_PRESSED:
            // Switch to shutdown screen
            lv_disp_load_scr(ui_shutdown_screen);
            
            // Start arc animation
            lv_anim_init(&arc_anim);
            lv_anim_set_var(&arc_anim, ui_Arc1);
            lv_anim_set_exec_cb(&arc_anim, set_arc_value);
            lv_anim_set_time(&arc_anim, 2000);  // 2 seconds to fill
            lv_anim_set_values(&arc_anim, 0, 100);
            lv_anim_start(&arc_anim);
            arc_animation_active = true;
            break;

        case BUTTON_EVENT_RELEASED:
            if (arc_animation_active) {
                // If released before full, cancel sleep
                lv_anim_del(ui_Arc1, set_arc_value);
                lv_arc_set_value(ui_Arc1, 0);
                arc_animation_active = false;
                lv_disp_load_scr(ui_home_screen);
            }
            break;

        case BUTTON_EVENT_LONG_PRESS:
            // Do nothing - we're handling sleep through the arc animation
            break;

        case BUTTON_EVENT_DOUBLE_PRESS:
            // Cancel any ongoing sleep animation and return to home screen
            if (arc_animation_active) {
                lv_anim_del(ui_Arc1, set_arc_value);
                lv_arc_set_value(ui_Arc1, 0);
                arc_animation_active = false;
                lv_disp_load_scr(ui_home_screen);
            }
            break;
    }
}

void sleep_init(void) {
    button_config_t config = {
        .gpio_num = MAIN_BUTTON_GPIO,
        .long_press_time_ms = SLEEP_TIMEOUT_MS,
        .double_press_time_ms = 300,
        .active_low = true
    };

    ESP_ERROR_CHECK(button_init(&config));
    button_register_callback(sleep_button_callback, NULL);

    ESP_ERROR_CHECK(esp_sleep_enable_gpio_wakeup());
    last_activity_time = xTaskGetTickCount();
}

void sleep_start_monitoring(void) {
    button_start_monitoring();
}

void sleep_reset_inactivity_timer(void)
{
    TickType_t current_time = xTaskGetTickCount();

    // Only reset if enough time has passed since last reset
    if ((current_time - last_reset_time) * portTICK_PERIOD_MS >= RESET_DEBOUNCE_TIME_MS) {
        last_activity_time = current_time;
        last_reset_time = current_time;
    }
}

void sleep_check_inactivity(bool is_ble_connected)
{
    TickType_t current_time = xTaskGetTickCount();
    TickType_t elapsed_time = (current_time - last_activity_time) * portTICK_PERIOD_MS;

    // Check if we should go to sleep (if inactive and not connected)
    if (elapsed_time > INACTIVITY_TIMEOUT_MS && !is_ble_connected) {
        ESP_LOGI(TAG, "System inactive for %lu ms and no BLE connection. Entering deep sleep.",
                 elapsed_time);

        // Configure wakeup on button press (transition from HIGH to LOW)
        ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(1ULL << MAIN_BUTTON_GPIO,
                                                      ESP_GPIO_WAKEUP_GPIO_LOW));

        // Enter deep sleep
        esp_deep_sleep_start();
    }
}