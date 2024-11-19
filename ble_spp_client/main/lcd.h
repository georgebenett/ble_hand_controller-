#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "adc.h"  // To get access to adc_get_queue()

#ifdef __cplusplus
extern "C" {
#endif

// Display resolution
#define LCD_H_RES 320
#define LCD_V_RES 240

// Color definitions in RGB565 format
#define COLOR_BLACK       0xFFFF
#define COLOR_WHITE       0x0000
#define COLOR_RED         0xF800
#define COLOR_GREEN       0x07E0
#define COLOR_BLUE        0x001F
#define COLOR_YELLOW      0xFFE0
#define COLOR_CYAN        0x07FF
#define COLOR_MAGENTA     0xF81F

// Function declarations
esp_err_t lcd_init(void);
esp_err_t lcd_draw_pixel(int x, int y, uint16_t color);
esp_err_t lcd_fill_rect(int x, int y, int width, int height, uint16_t color);
esp_err_t lcd_clear(uint16_t color);
esp_err_t lcd_draw_line(int x1, int y1, int x2, int y2, uint16_t color);
void lcd_draw_digit(int16_t x, int16_t y, uint8_t digit, uint16_t color);
void lcd_draw_number(int16_t x, int16_t y, uint32_t number, uint16_t color);
void lcd_init_display_task(void);

#ifdef __cplusplus
}
#endif 