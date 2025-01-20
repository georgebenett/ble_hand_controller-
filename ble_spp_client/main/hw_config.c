#include "hw_config.h"
#include "esp_log.h"
#include "driver/gpio.h"
#define TAG "HW_CONFIG"

void hw_config_init(void) {
    // Configure vibration motor pin
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << VIBER_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure vibration motor pin");
    }

    // Initialize vibration motor to off state
    gpio_set_level(VIBER_PIN, 0);
}