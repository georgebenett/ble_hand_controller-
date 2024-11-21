#include "adc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>
#include <stdint.h>

static const char *TAG = "ADC";
static adc_oneshot_unit_handle_t adc1_handle;
static adc_oneshot_unit_init_cfg_t init_config1;
static adc_oneshot_chan_cfg_t config;
static QueueHandle_t adc_display_queue = NULL;
static uint32_t latest_adc_value = 0;

esp_err_t adc_init(void)
{
    // Create queue first
    adc_display_queue = xQueueCreate(10, sizeof(uint32_t));
    if (adc_display_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create queue");
        return ESP_FAIL;
    }

    // ADC1 init configuration
    init_config1.unit_id = ADC_UNIT_1;
    init_config1.ulp_mode = ADC_ULP_MODE_DISABLE;
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // Channel configuration
    config.atten = ADC_ATTEN_DB_12;
    config.bitwidth = ADC_BITWIDTH_12;
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, THROTTLE_PIN, &config));
    
    return ESP_OK;
}

int32_t adc_read_value(void)
{
    int adc_raw;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, THROTTLE_PIN, &adc_raw));

    // Clamp the ADC value to the valid range
    if (adc_raw < ADC_INPUT_MIN_VALUE) adc_raw = ADC_INPUT_MIN_VALUE;
    if (adc_raw > ADC_INPUT_MAX_VALUE) adc_raw = ADC_INPUT_MAX_VALUE;
    
    int32_t mapped = (adc_raw - ADC_INPUT_MIN_VALUE) * (ADC_OUTPUT_MAX_VALUE - ADC_OUTPUT_MIN_VALUE) /
                     (ADC_INPUT_MAX_VALUE - ADC_INPUT_MIN_VALUE) + ADC_OUTPUT_MIN_VALUE;

    return mapped;
}

static void adc_task(void *pvParameters) {
    while (1) {
        uint32_t adc_value = adc_read_value();
        latest_adc_value = adc_value;  // Store the latest value
        xQueueSend(adc_display_queue, &adc_value, 0);
        vTaskDelay(pdMS_TO_TICKS(ADC_SAMPLING_TICKS));
    }
}

void adc_start_task(void) {
    
    // Create the ADC reading task
    xTaskCreate(adc_task, "adc_task", 4096, NULL, 5, NULL);
}

QueueHandle_t adc_get_queue(void)
{
    return adc_display_queue;
}

// Add this function to get the latest ADC value
uint32_t adc_get_latest_value(void) {
    return latest_adc_value;
} 