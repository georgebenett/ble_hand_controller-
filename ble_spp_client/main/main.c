#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "ble_spp_client.h"
#include "adc.h"
#include "lcd.h"

#define TAG "MAIN"

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

    // Initialize LCD
    ESP_ERROR_CHECK(lcd_init());
    
    // Test LCD
    lcd_clear(COLOR_WHITE);
    lcd_fill_rect(10, 10, 100, 20, COLOR_RED);
    lcd_draw_line(0, 0, LCD_H_RES-1, LCD_V_RES-1, COLOR_GREEN);

    // Initialize ADC
    ESP_ERROR_CHECK(adc_init());
    
    // Start ADC task for periodic readings
    adc_start_task();
    
    // Initialize the SPP client demo
    spp_client_demo_init();
    
    ESP_LOGI(TAG, "Initialization complete");
} 