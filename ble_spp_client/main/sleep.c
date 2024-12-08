#include "sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "lcd.h"

#define TAG "SLEEP"

static TickType_t last_activity_time;
static TaskHandle_t sleep_monitor_task_handle = NULL;
static TickType_t last_reset_time = 0;
#define RESET_DEBOUNCE_TIME_MS 2000  // Minimum time between resets (2s)

static void init_sleep_gpio(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << SLEEP_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // Enable wake up from GPIO (active low)
    ESP_ERROR_CHECK(gpio_wakeup_enable(SLEEP_PIN, GPIO_INTR_LOW_LEVEL));
    ESP_ERROR_CHECK(esp_sleep_enable_gpio_wakeup());
}

static void check_sleep_conditions(void *pvParameters)
{
    TickType_t pin_low_time = 0;
    bool pin_was_low = false;

    while (1) {
        int pin_level = gpio_get_level(SLEEP_PIN);

        if (pin_level == 0) {  // Button is pressed (active low)
            if (!pin_was_low) {
                pin_was_low = true;
                pin_low_time = xTaskGetTickCount();
            } else {
                // Calculate how long the button has been held
                uint32_t elapsed_ms = (xTaskGetTickCount() - pin_low_time) * portTICK_PERIOD_MS;

                // Show progress only while holding (up to SLEEP_TIMEOUT_MS)
                if (elapsed_ms < SLEEP_TIMEOUT_MS) {
                    uint8_t progress = (elapsed_ms * 100) / SLEEP_TIMEOUT_MS;
                    lcd_show_loading_bar(progress);
                }

                // Check if button has been held for timeout period
                if (elapsed_ms >= SLEEP_TIMEOUT_MS) {
                    ESP_LOGI(TAG, "Entering deep sleep mode");
                    lcd_show_loading_bar(100);  // Show full progress

                    // Wait for button release before sleeping
                    while (gpio_get_level(SLEEP_PIN) == 0) {
                        vTaskDelay(pdMS_TO_TICKS(10));
                    }

                    lcd_hide_loading_bar();  // Hide the loading bar before sleep
                    lcd_reset_loading_bar();  // Reset the pointer

                    // Configure wakeup on button press (active low)
                    ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(1ULL << SLEEP_PIN, ESP_GPIO_WAKEUP_GPIO_LOW));

                    // Enter deep sleep
                    esp_deep_sleep_start();
                }
            }
        } else {
            if (pin_was_low) {
                lcd_hide_loading_bar();  // Hide the loading bar when button is released
                lcd_reset_loading_bar();  // Reset the pointer
            }
            pin_was_low = false;
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void sleep_init(void)
{
    init_sleep_gpio();
    last_activity_time = xTaskGetTickCount();
}

void sleep_start_monitoring(void)
{
    xTaskCreate(check_sleep_conditions, "sleep_monitor", 2048, NULL, 3, &sleep_monitor_task_handle);
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
        ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(1ULL << SLEEP_PIN,
                                                       ESP_GPIO_WAKEUP_GPIO_LOW));

        // Enter deep sleep
        esp_deep_sleep_start();
    }
} 