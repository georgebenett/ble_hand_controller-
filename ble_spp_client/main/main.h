#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

extern SemaphoreHandle_t lvgl_mutex;
extern SemaphoreHandle_t ble_data_mutex;
extern SemaphoreHandle_t adc_mutex;

void init_system_mutexes(void);