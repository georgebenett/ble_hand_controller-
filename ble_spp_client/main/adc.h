#ifndef ADC_H
#define ADC_H

#include <stdint.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

esp_err_t adc_init(void);
int32_t adc_read_value(void);
void adc_start_task(void);
QueueHandle_t adc_get_queue(void);

#endif // ADC_H 