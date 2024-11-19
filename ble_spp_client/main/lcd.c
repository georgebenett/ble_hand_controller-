#include "lcd.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define TAG "LCD"

// Pin definitions
#define PIN_MOSI GPIO_NUM_10
#define PIN_SCLK GPIO_NUM_8
#define PIN_CS   GPIO_NUM_6
#define PIN_DC   GPIO_NUM_7
#define PIN_RST  GPIO_NUM_21

#define LCD_PIXEL_CLOCK_HZ (40 * 1000 * 1000) // 40MHz
#define LCD_HOST    SPI2_HOST

static esp_lcd_panel_io_handle_t io_handle = NULL;
static esp_lcd_panel_handle_t panel_handle = NULL;

esp_err_t lcd_init(void)
{
    esp_err_t ret = ESP_OK;

    ESP_LOGI(TAG, "Initialize SPI bus");
    spi_bus_config_t buscfg = {
        .sclk_io_num = PIN_SCLK,
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * LCD_V_RES * 2 + 8
    };
    ret = spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG, "Initialize panel IO");
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_DC,
        .cs_gpio_num = PIN_CS,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle);
    if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG, "Initialize ST7789 panel driver");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_RST,
        .rgb_endian = LCD_RGB_ENDIAN_BGR,
        .bits_per_pixel = 16,
    };
    ret = esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle);
    if (ret != ESP_OK) return ret;

    ret = esp_lcd_panel_reset(panel_handle);
    if (ret != ESP_OK) return ret;

    ret = esp_lcd_panel_init(panel_handle);
    if (ret != ESP_OK) return ret;

    // Set orientation
    ret = esp_lcd_panel_swap_xy(panel_handle, true);
    if (ret != ESP_OK) return ret;

    ret = esp_lcd_panel_mirror(panel_handle, true, false);
    if (ret != ESP_OK) return ret;

    ret = esp_lcd_panel_disp_on_off(panel_handle, true);
    return ret;
}

esp_err_t lcd_draw_pixel(int x, int y, uint16_t color)
{
    return esp_lcd_panel_draw_bitmap(panel_handle, x, y, x + 1, y + 1, &color);
}

esp_err_t lcd_fill_rect(int x, int y, int width, int height, uint16_t color)
{
    uint16_t *buffer = heap_caps_malloc(width * sizeof(uint16_t), MALLOC_CAP_DMA);
    if (!buffer) {
        return ESP_ERR_NO_MEM;
    }

    // Fill buffer with color
    for (int i = 0; i < width; i++) {
        buffer[i] = color;
    }

    // Draw rectangle line by line
    for (int row = y; row < y + height; row++) {
        esp_lcd_panel_draw_bitmap(panel_handle, x, row, x + width, row + 1, buffer);
    }

    free(buffer);
    return ESP_OK;
}

esp_err_t lcd_clear(uint16_t color)
{
    return lcd_fill_rect(0, 0, LCD_H_RES, LCD_V_RES, color);
}

esp_err_t lcd_draw_line(int x1, int y1, int x2, int y2, uint16_t color)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = x1 < x2 ? 1 : -1;
    int sy = y1 < y2 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    int e2;

    while (1) {
        lcd_draw_pixel(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        e2 = err;
        if (e2 > -dx) {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dy) {
            err += dx;
            y1 += sy;
        }
    }
    return ESP_OK;
} 