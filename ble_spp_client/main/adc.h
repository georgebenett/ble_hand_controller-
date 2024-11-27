#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#define ADC_SAMPLING_TICKS 20
#define THROTTLE_PIN ADC_CHANNEL_2

#define ADC_INPUT_MAX_VALUE 3230
#define ADC_INPUT_MIN_VALUE 1320

#define ADC_OUTPUT_MAX_VALUE 255
#define ADC_OUTPUT_MIN_VALUE 0

esp_err_t adc_init(void);
int32_t adc_read_value(void);
void adc_start_task(void);
QueueHandle_t adc_get_queue(void);
uint32_t adc_get_latest_value(void);
uint8_t map_adc_value(uint32_t adc_value);

#endif // ADC_H 