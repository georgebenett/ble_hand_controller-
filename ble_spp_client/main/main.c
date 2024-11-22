#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "ble_spp_client.h"
#include "adc.h"
#include "hagl.h"
#include "hagl_hal.h"
#include "font5x8.h"
#include "driver/gpio.h"
#include <driver/spi_master.h>

#define TAG "MAIN"

// Add these function declarations at the top, after includes
void lcd_update_task(void *pvParameters);
esp_err_t lcd_init(void);  // You'll need to implement this
int adc_get_value(void);   // You'll need to implement this

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
    // Use the global backend instead of trying to get it
    hagl_backend_t *backend = display_backend;
    
    // Define some basic colors (RGB565 format)
    uint16_t RED = 0x001F;;     // Full red
    uint16_t GREEN = 0x07E0;   // Full green
    uint16_t BLUE = 0xF800;    // Full blue
    uint16_t WHITE = 0xFFFF;   // White
    uint16_t YELLOW = 0xFFE0;  // Yellow
    
    while (1) {
        // Clear the screen with black
        hagl_clear(backend);
        
        // Draw several circles with different colors and positions
        hagl_fill_circle(backend, 60, 60, 30, RED);          // Red circle
        hagl_fill_circle(backend, 120, 60, 25, GREEN);       // Green circle
        hagl_fill_circle(backend, 180, 60, 20, BLUE);        // Blue circle
        
        // Draw some outlined circles
        hagl_draw_circle(backend, 60, 120, 35, WHITE);       // White circle outline
        hagl_draw_circle(backend, 120, 120, 30, YELLOW);     // Yellow circle outline
        
        // Draw a pattern of small filled circles
        for(int i = 0; i < 5; i++) {
            hagl_fill_circle(backend, 30 + (i * 40), 180, 10, WHITE);
        }
        
        // Flush to display
        hagl_flush(backend);
        
        // Wait before next update
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
} 