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
static bool adc_initialized = false;
static int error_count = 0;
static const int MAX_ERRORS = 5;

// Add this function prototype
void adc_deinit(void);

esp_err_t adc_init(void)
{
    if (adc_initialized) {
        ESP_LOGI(TAG, "ADC already initialized");
        return ESP_OK;
    }

    esp_err_t ret;

    // Create queue first
    adc_display_queue = xQueueCreate(10, sizeof(uint32_t));
    if (adc_display_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create queue");
        return ESP_FAIL;
    }

    // ADC1 init configuration
    init_config1.unit_id = ADC_UNIT_1;
    init_config1.ulp_mode = ADC_ULP_MODE_DISABLE;
    ret = adc_oneshot_new_unit(&init_config1, &adc1_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ADC unit initialization failed");
        return ret;
    }

    // Channel configuration
    config.atten = ADC_ATTEN_DB_12;
    config.bitwidth = ADC_BITWIDTH_12;
    ret = adc_oneshot_config_channel(adc1_handle, THROTTLE_PIN, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ADC channel configuration failed");
        return ret;
    }

    adc_initialized = true;
    return ESP_OK;
}

int32_t adc_read_value(void)
{
    if (!adc_initialized || !adc1_handle) {
        ESP_LOGE(TAG, "ADC not properly initialized");
        return -1;
    }

    // Take multiple readings and average
    const int NUM_SAMPLES = 5;
    int32_t sum = 0;
    int valid_samples = 0;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        int adc_raw = 0;
        esp_err_t ret = adc_oneshot_read(adc1_handle, THROTTLE_PIN, &adc_raw);

        if (ret == ESP_OK) {
            sum += adc_raw;
            valid_samples++;
        }

        // Small delay between samples
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    return valid_samples > 0 ? (sum / valid_samples) : -1;
}

static void adc_task(void *pvParameters) {
    while (1) {
        uint32_t adc_value = adc_read_value();
        if (adc_value == -1) {
            error_count++;
            if (error_count >= MAX_ERRORS) {
                ESP_LOGE(TAG, "Too many ADC errors, attempting re-initialization");
                adc_deinit();
                vTaskDelay(pdMS_TO_TICKS(100));
                if (adc_init() == ESP_OK) {
                    error_count = 0;
                }
            }
            vTaskDelay(pdMS_TO_TICKS(100));  // Wait before retry
            continue;
        }
        error_count = 0;  // Reset error count on successful read

        uint8_t mapped_value = map_adc_value(adc_value);
        latest_adc_value = mapped_value;
        xQueueSend(adc_display_queue, &mapped_value, 0);
        vTaskDelay(pdMS_TO_TICKS(ADC_SAMPLING_TICKS));
    }
}

void adc_start_task(void) {
    esp_err_t ret = adc_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ADC initialization failed, not starting task");
        return;
    }

    // Add delay after initialization
    vTaskDelay(pdMS_TO_TICKS(100));

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

void adc_deinit(void)
{
    if (!adc_initialized) {
        return;
    }

    if (adc1_handle) {
        adc_oneshot_del_unit(adc1_handle);
        adc1_handle = NULL;
    }

    if (adc_display_queue) {
        vQueueDelete(adc_display_queue);
        adc_display_queue = NULL;
    }

    adc_initialized = false;
}

uint8_t map_adc_value(uint32_t adc_value) {
    // Constrain input value to the defined range
    if (adc_value < ADC_INPUT_MIN_VALUE) {
        adc_value = ADC_INPUT_MIN_VALUE;
    }
    if (adc_value > ADC_INPUT_MAX_VALUE) {
        adc_value = ADC_INPUT_MAX_VALUE;
    }

    // Perform the mapping
    return (uint8_t)((adc_value - ADC_INPUT_MIN_VALUE) *
           (ADC_OUTPUT_MAX_VALUE - ADC_OUTPUT_MIN_VALUE) /
           (ADC_INPUT_MAX_VALUE - ADC_INPUT_MIN_VALUE) +
           ADC_OUTPUT_MIN_VALUE);
}