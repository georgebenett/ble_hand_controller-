#ifndef HW_CONFIG_H
#define HW_CONFIG_H

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

#define CORE_0 0
#define CORE_1 1

/* ADC Configuration */
#define THROTTLE_PIN             ADC_CHANNEL_9 // adc1_ch9->gpio_num_10
#define HALL_SENSOR_VDD_PIN      GPIO_NUM_14

/* Battery Configuration */
#define BATTERY_ADC_CHANNEL      ADC_CHANNEL_3
#define BATTERY_CHARGING_PIN     GPIO_NUM_3

/* Viber configuration */
#define VIBER_PIN                GPIO_NUM_46

/* Button Configuration */
#define MAIN_BUTTON_GPIO GPIO_NUM_8


#endif // HW_CONFIG_H
