#include "button.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>


#define TAG "BUTTON"
#define DEBOUNCE_TIME_MS 20
#define TASK_STACK_SIZE 2048
#define TASK_PRIORITY 3


static button_config_t button_cfg;
static button_state_t current_state = BUTTON_IDLE;
static button_callback_t event_callback = NULL;
static void* callback_user_data = NULL;
static TickType_t press_start_time = 0;
static TickType_t last_release_time = 0;
static bool first_press_registered = false;
static TaskHandle_t button_task_handle = NULL;

static void button_monitor_task(void* pvParameters) {
    bool last_reading = !button_cfg.active_low;
    bool button_pressed = false;

    while (1) {
        bool current_reading = gpio_get_level(button_cfg.gpio_num);
        if (button_cfg.active_low) {
            current_reading = !current_reading;
        }

        if (current_reading != last_reading) {
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_TIME_MS));
            current_reading = gpio_get_level(button_cfg.gpio_num);
            if (button_cfg.active_low) {
                current_reading = !current_reading;
            }
        }

        if (current_reading && !button_pressed) {
            press_start_time = xTaskGetTickCount();
            button_pressed = true;
            current_state = BUTTON_PRESSED;
            if (event_callback) {
                event_callback(BUTTON_EVENT_PRESSED, callback_user_data);
            }
        } else if (!current_reading && button_pressed) {
            button_pressed = false;
            uint32_t press_duration = (xTaskGetTickCount() - press_start_time) * portTICK_PERIOD_MS;

            if (press_duration >= button_cfg.long_press_time_ms) {
                current_state = BUTTON_LONG_PRESS;
                if (event_callback) {
                    event_callback(BUTTON_EVENT_LONG_PRESS, callback_user_data);
                }
            } else {
                TickType_t current_time = xTaskGetTickCount();
                if (first_press_registered &&
                    (current_time - last_release_time) * portTICK_PERIOD_MS < button_cfg.double_press_time_ms) {
                    current_state = BUTTON_DOUBLE_PRESS;
                    first_press_registered = false;
                    if (event_callback) {
                        event_callback(BUTTON_EVENT_DOUBLE_PRESS, callback_user_data);
                    }
                } else {
                    first_press_registered = true;
                    last_release_time = current_time;
                    if (event_callback) {
                        event_callback(BUTTON_EVENT_RELEASED, callback_user_data);
                    }
                }
            }
            current_state = BUTTON_IDLE;
        }

        last_reading = current_reading;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

esp_err_t button_init(const button_config_t* config) {
    if (config == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // Store configuration
    memcpy(&button_cfg, config, sizeof(button_config_t));


    // Configure GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << config->gpio_num),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = config->active_low ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .pull_down_en = config->active_low ? GPIO_PULLDOWN_DISABLE : GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    return gpio_config(&io_conf);
}

void button_register_callback(button_callback_t callback, void* user_data) {
    event_callback = callback;
    callback_user_data = user_data;
}

button_state_t button_get_state(void) {
    return current_state;
}

uint32_t button_get_press_duration_ms(void) {
    if (current_state == BUTTON_IDLE) {
        return 0;
    }
    return (xTaskGetTickCount() - press_start_time) * portTICK_PERIOD_MS;
}

void button_start_monitoring(void) {
    xTaskCreate(button_monitor_task, "button_monitor", TASK_STACK_SIZE,
                NULL, TASK_PRIORITY, &button_task_handle);
}

void button_event_handler(button_event_t event) {
    switch(event) {
        case BUTTON_EVENT_PRESSED:
            ESP_LOGI("BUTTON", "Button pressed");
            break;
        case BUTTON_EVENT_RELEASED:
            ESP_LOGI("BUTTON", "Button released");
            break;
        case BUTTON_EVENT_LONG_PRESS:
            ESP_LOGI("BUTTON", "Long press detected");
            break;
        case BUTTON_EVENT_DOUBLE_PRESS:
            ESP_LOGI("BUTTON", "Double press detected");
            break;
        default:
            ESP_LOGI("BUTTON", "Unknown button event: %d", event);
            break;
    }
}