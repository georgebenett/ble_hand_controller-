#include "sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lcd.h"
#include "ui/ui.h"
#include "lvgl.h"
#include "esp_sleep.h"
#include "driver/rtc_io.h"
#include "soc/rtc.h"
#include "esp_task_wdt.h"
#include "hw_config.h"
#include "ble_spp_client.h"


#define TAG "SLEEP"

static TickType_t last_activity_time;
static TickType_t last_reset_time = 0;
#define RESET_DEBOUNCE_TIME_MS 2000

static lv_anim_t arc_anim;
static bool arc_animation_active = false;

static void set_bar_value(void * obj, int32_t v)
{
    lv_bar_set_value(obj, v, LV_ANIM_OFF);

    // If we reach 100%, trigger sleep immediately
    if (v >= 100) {
        vTaskDelay(pdMS_TO_TICKS(250));
        enter_deep_sleep();
    }
}

static void sleep_button_callback(button_event_t event, void* user_data) {
    static bool long_press_triggered = false;

    switch(event) {
        case BUTTON_EVENT_PRESSED:
            long_press_triggered = false;
            break;

        case BUTTON_EVENT_RELEASED:
            if (arc_animation_active) {
                // If released before full, cancel sleep
                lv_anim_del(ui_Bar4, set_bar_value);
                lv_bar_set_value(ui_Bar4, 0, LV_ANIM_OFF);
                arc_animation_active = false;
                lv_disp_load_scr(ui_home_screen);
            }
            long_press_triggered = false;
            break;

        case BUTTON_EVENT_LONG_PRESS:
            if (!long_press_triggered) {
                long_press_triggered = true;
                // Switch to shutdown screen
                lv_disp_load_scr(ui_shutdown_screen);
                // Start bar animation
                lv_anim_init(&arc_anim);
                lv_anim_set_var(&arc_anim, ui_Bar4);
                lv_anim_set_exec_cb(&arc_anim, set_bar_value);
                lv_anim_set_time(&arc_anim, 2000);  // 2 seconds to fill
                lv_anim_set_values(&arc_anim, 0, 100);
                lv_anim_start(&arc_anim);
                arc_animation_active = true;
            }
            break;

        case BUTTON_EVENT_DOUBLE_PRESS:
            break;
    }
}

static void sleep_monitor_task(void *pvParameters) {
    while (1) {
        sleep_check_inactivity(is_connect);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void sleep_init(void) {
    button_config_t config = {
        .gpio_num = MAIN_BUTTON_GPIO,
        .long_press_time_ms = BUTTON_LONG_PRESS_TIME_MS,
        .double_press_time_ms = BUTTON_DOUBLE_PRESS_TIME_MS,
        .active_low = true
    };

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << HALL_SENSOR_VDD_PIN) | (1ULL << HALL_SENSOR_GND_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Set initial state - power on the sensor
    gpio_set_level(HALL_SENSOR_VDD_PIN, 1);  // VDD on
    gpio_set_level(HALL_SENSOR_GND_PIN, 0);  // GND off

    ESP_ERROR_CHECK(button_init(&config));
    button_register_callback(sleep_button_callback, NULL);

    last_activity_time = xTaskGetTickCount();

    xTaskCreatePinnedToCore(sleep_monitor_task, "sleep_monitor", 2048, NULL, 4, NULL, CORE_1);
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
        enter_deep_sleep();
    }
}

void enter_deep_sleep(void) {

    // Disable task watchdog (only if it was enabled)
    #if CONFIG_ESP_TASK_WDT_EN
        esp_task_wdt_deinit();
    #endif

    //power down lcd
    gpio_set_level(TFT_GND_PIN, 0);
    gpio_set_level(TFT_VDD_PIN, 0);

    // Power down Hall sensor
    gpio_set_level(HALL_SENSOR_VDD_PIN, 0);
    gpio_set_level(HALL_SENSOR_GND_PIN, 0);

    // Configure RTC GPIO for wake-up
    rtc_gpio_init(MAIN_BUTTON_GPIO);
    rtc_gpio_set_direction(MAIN_BUTTON_GPIO, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pullup_en(MAIN_BUTTON_GPIO);
    rtc_gpio_pulldown_dis(MAIN_BUTTON_GPIO);
    rtc_gpio_hold_en(MAIN_BUTTON_GPIO);


    while (gpio_get_level(MAIN_BUTTON_GPIO) == 0) { //wait for button to be released
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    // Add debounce delay after release
    vTaskDelay(pdMS_TO_TICKS(100));
    // Enable wake-up on low level (button press)
    ESP_ERROR_CHECK(esp_sleep_enable_ext0_wakeup(MAIN_BUTTON_GPIO, 0));

    // Enter deep sleep
    ESP_LOGI(TAG, "Entering deep sleep");

    esp_deep_sleep_start();
}