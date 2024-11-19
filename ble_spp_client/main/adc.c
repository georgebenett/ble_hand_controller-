#include "adc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "lcd.h"
#include <string.h>
#include <stdint.h>

static const char *TAG = "ADC";
static adc_oneshot_unit_handle_t adc1_handle;
static adc_oneshot_unit_init_cfg_t init_config1;
static adc_oneshot_chan_cfg_t config;

#define ADC_TAG "ADC"
#define LCD_REFRESH_PERIOD_MS (uint32_t)20

static QueueHandle_t adc_display_queue;
static TaskHandle_t lcd_task_handle;

esp_err_t adc_init(void)
{
    // ADC1 init configuration
    init_config1.unit_id = ADC_UNIT_1;
    init_config1.ulp_mode = ADC_ULP_MODE_DISABLE;
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // Channel configuration (GPIO2 is connected to ADC1 channel 2)
    config.atten = ADC_ATTEN_DB_12;
    config.bitwidth = ADC_BITWIDTH_12;
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANNEL_2, &config));
    
    // Create queue for ADC values
    adc_display_queue = xQueueCreate(5, sizeof(uint32_t));
    
    return ESP_OK;
}

int32_t adc_read_value(void)
{
    int32_t adc_raw;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_2, &adc_raw));
    
    // Convert to voltage (rough approximation, use esp_adc_cal for better accuracy)
    int32_t voltage_mv = (adc_raw * 3300) / 4095;
    
    //ESP_LOGI(TAG, "ADC Raw: %d, Voltage: %dmV", adc_raw, voltage_mv);
    return adc_raw;
}

static void lcd_display_task(void *pvParameters) {
    uint32_t adc_value;
    char buf[32];
    
    // Get LCD dimensions from the LCD header
    const int32_t center_x = LCD_H_RES/2;
    const int32_t center_y = LCD_V_RES/2;
    const int32_t clear_width = 100;
    const int32_t clear_height = 40;
    
    while (1) {
        if (xQueueReceive(adc_display_queue, &adc_value, pdMS_TO_TICKS(LCD_REFRESH_PERIOD_MS)) == pdTRUE) {
            // Clear the center area of display
            lcd_fill_rect(
                (int16_t)(center_x - clear_width/2),  // x position
                (int16_t)(center_y - clear_height/2),  // y position
                (uint16_t)clear_width,                // width
                (uint16_t)clear_height,               // height
                COLOR_WHITE
            );
            
            // Format ADC value
            snprintf(buf, sizeof(buf), "ADC: %lu", adc_value);
            
            // Draw a rectangle to indicate the value
            uint32_t value_width = (adc_value * clear_width) / 4095;  // Scale to display width
            lcd_fill_rect(
                (int16_t)(center_x - clear_width/2),
                (int16_t)(center_y - 10),
                (uint16_t)value_width,
                (uint16_t)20,
                COLOR_BLACK
            );
        }
    }
}

static void adc_task(void *pvParameters) {
    while (1) {
        uint32_t adc_value = adc_read_value();
        xQueueSend(adc_display_queue, &adc_value, 0);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void adc_start_task(void) {
    // Create the display task
    xTaskCreate(lcd_display_task, "lcd_display", 4096, NULL, 5, &lcd_task_handle);
    
    // Create the ADC reading task
    xTaskCreate(adc_task, "adc_task", 4096, NULL, 5, NULL);
} 