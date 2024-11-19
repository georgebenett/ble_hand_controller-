#include "adc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "ADC";
static adc_oneshot_unit_handle_t adc1_handle;
static adc_oneshot_unit_init_cfg_t init_config1;
static adc_oneshot_chan_cfg_t config;

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
    
    return ESP_OK;
}

int adc_read_value(void)
{
    int adc_raw;
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANNEL_2, &adc_raw));
    
    // Convert to voltage (rough approximation, use esp_adc_cal for better accuracy)
    int voltage_mv = (adc_raw * 3300) / 4095;
    
    ESP_LOGI(TAG, "ADC Raw: %d, Voltage: %dmV", adc_raw, voltage_mv);
    return adc_raw;
}

static void adc_task(void *pvParameters)
{
    while(1) {
        adc_read_value();
        vTaskDelay(pdMS_TO_TICKS(1000));  // Read every second
    }
}

void adc_start_task(void)
{
    xTaskCreate(adc_task, "adc_task", 2048, NULL, 5, NULL);
} 