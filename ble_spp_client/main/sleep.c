#include "sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lcd.h"

#define TAG "SLEEP"

static TickType_t last_activity_time;
static TickType_t last_reset_time = 0;
#define RESET_DEBOUNCE_TIME_MS 2000

static void sleep_button_callback(button_event_t event, void* user_data) {
    // First log all button events
    if (event == BUTTON_EVENT_LONG_PRESS) {
        ESP_LOGI(TAG, "Long press detected - Entering deep sleep mode");
        ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(1ULL << MAIN_BUTTON_GPIO,
                                                          ESP_GPIO_WAKEUP_GPIO_LOW));
        vTaskDelay(pdMS_TO_TICKS(100));
        esp_deep_sleep_start();
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

        // Configure wakeup on button press (active low)
        ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(1ULL << MAIN_BUTTON_GPIO,
                                                       ESP_GPIO_WAKEUP_GPIO_LOW));

        // Enter deep sleep
        esp_deep_sleep_start();
    }
}