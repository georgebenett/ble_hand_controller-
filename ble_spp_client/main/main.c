#include <stdio.h>
#include <wchar.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "ble_spp_client.h"
#include "adc.h"
#include "hagl.h"
#include "hagl_hal.h"
#include "font6x9.h"
#include "driver/gpio.h"
#include <driver/spi_master.h>

#define TAG "MAIN"

// Add these function declarations at the top, after includes
void lcd_update_task(void *pvParameters);
esp_err_t lcd_init(void);

// Add at the top with other global declarations
static hagl_backend_t *display_backend = NULL;

esp_err_t lcd_init(void)
{
    // Initialize HAGL and get the display backend
    display_backend = hagl_init();

    if (display_backend == NULL) {
        ESP_LOGE(TAG, "Failed to initialize display");
        return ESP_FAIL;
    }

    // Clear the screen initially
    hagl_clear(display_backend);
    hagl_flush(display_backend);

    ESP_LOGI(TAG, "Display initialized successfully");
    return ESP_OK;
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Application");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize LCD and HAGL
    ESP_ERROR_CHECK(lcd_init());

    // Initialize ADC and start tasks
    ESP_ERROR_CHECK(adc_init());
    adc_start_task();

    // Create task for LCD update
    xTaskCreate(lcd_update_task, "lcd_update", 4096, NULL, 5, NULL);

    // Initialize the SPP client demo
    spp_client_demo_init();

    ESP_LOGI(TAG, "Initialization complete");
}

// Update the lcd_update_task function
void lcd_update_task(void *pvParameters)
{
    hagl_backend_t *backend = display_backend;
    uint16_t WHITE = 0xFFFF;
    uint16_t BLACK = 0x0000;
    wchar_t adc_str[32];
    int last_adc_value = -1;
    wchar_t last_adc_str[32] = L"";

    while (1) {
        // Map ADC value and store it
        int mapped_value = map_adc_value(adc_read_value());

        if (mapped_value != last_adc_value) {
            // Erase old value with black text
            hagl_put_text(backend, last_adc_str, 150, 150, BLACK, font6x9);

            // Format string with just the mapped value
            swprintf(adc_str, sizeof(adc_str)/sizeof(wchar_t),
                    L"%d%", mapped_value);

            // Draw new value in white
            hagl_put_text(backend, adc_str, 150, 150, WHITE, font6x9);

            hagl_flush(backend);
            wcscpy(last_adc_str, adc_str);
            last_adc_value = mapped_value;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}