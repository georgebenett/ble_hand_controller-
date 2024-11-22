#include <stdio.h>
#include <wchar.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "ble_spp_client.h"
#include "adc.h"
#include "driver/gpio.h"
#include <driver/spi_master.h>

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
    
    // Initialize ADC and start tasks
    ESP_ERROR_CHECK(adc_init());
    adc_start_task();
    
    // Initialize the SPP client demo
    spp_client_demo_init();
    
    ESP_LOGI(TAG, "Initialization complete");
} 