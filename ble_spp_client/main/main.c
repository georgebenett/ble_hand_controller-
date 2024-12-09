#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "ble_spp_client.h"
#include "adc.h"
#include "lcd.h"
#include "driver/gpio.h"
#include "esp_sleep.h"
#include "sleep.h"
#include "menu.h"
#include "button.h"

#define TAG "MAIN"


extern bool is_connect;

static lv_obj_t *adc_label = NULL;
static char adc_str[32];


static void update_display_task(void *pvParameters) {
    TickType_t last_wake_time = xTaskGetTickCount();

    while (1) {
        float voltage = get_latest_voltage();
        int32_t rpm = get_latest_rpm();
        int quality = get_connection_quality();
        
        uint8_t calibrated_adc = adc_get_latest_value();
        menu_handle_navigation(calibrated_adc);

        snprintf(adc_str, sizeof(adc_str), "%.2fv\n%ld\n%d%%",
                voltage, rpm, quality);

        if (adc_label != NULL) {
            lv_label_set_text(adc_label, adc_str);
        }

        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(20));
    }
}


void app_main(void)
{
    ESP_LOGI(TAG, "Starting Application");

    // Initialize sleep module
    sleep_init();

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize ADC and start tasks
    ESP_ERROR_CHECK(adc_init());
    adc_start_task();

    // Wait for ADC calibration
    while (!adc_is_calibrated()) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Initialize BLE
    spp_client_demo_init();
    ESP_LOGI(TAG, "BLE Initialization complete");

    // Initialize LCD and LVGL
    lcd_init();

    // Create and configure display label
    adc_label = lcd_create_label("0");

    // Start display tasks
    lcd_start_tasks();

    // Create display update task
    xTaskCreate(update_display_task, "update_display", 4096, NULL, 4, NULL);

    // Start sleep monitoring
    sleep_start_monitoring();

    // Main task can now sleep
    while (1) {
        sleep_check_inactivity(is_connect);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

