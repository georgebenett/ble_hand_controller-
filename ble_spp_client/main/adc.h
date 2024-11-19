#ifndef ADC_H
#define ADC_H

#include "esp_err.h"

esp_err_t adc_init(void);
int adc_read_value(void);
void adc_start_task(void);

#endif // ADC_H 