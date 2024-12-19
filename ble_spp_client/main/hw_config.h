#ifndef HW_CONFIG_H
#define HW_CONFIG_H

#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"

/* ADC Configuration */
#define THROTTLE_PIN             ADC_CHANNEL_9 // adc1_ch9->gpio_num_10
#define BATTERY_ADC_CHANNEL      ADC_CHANNEL_3

#define HALL_SENSOR_GND_PIN      GPIO_NUM_13
#define HALL_SENSOR_VDD_PIN      GPIO_NUM_11


/* Button Configuration */
#define MAIN_BUTTON_GPIO GPIO_NUM_8

/* Display Configuration */
#define TFT_MOSI_PIN GPIO_NUM_40
#define TFT_SCLK_PIN GPIO_NUM_41
#define TFT_CS_PIN   GPIO_NUM_6
#define TFT_DC_PIN   GPIO_NUM_7
#define TFT_RST_PIN  GPIO_NUM_39

#define TFT_GND_PIN  GPIO_NUM_45
#define TFT_VDD_PIN  GPIO_NUM_47

#endif // HW_CONFIG_H