#include <stdio.h>
#include <wchar.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "ble_spp_client.h"
#include "adc.h"
#include "driver/gpio.h"
#include <driver/spi_master.h>
#include "lvgl.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "esp_assert.h"



#define TAG "MAIN"

// Add these buffer declarations before init_display()
static lv_color_t *buf1 = NULL;
static lv_color_t *buf2 = NULL;

// Define your display's SPI pins
#define TFT_MOSI_PIN GPIO_NUM_10
#define TFT_SCLK_PIN GPIO_NUM_8
#define TFT_CS_PIN   GPIO_NUM_6
#define TFT_DC_PIN   GPIO_NUM_7
#define TFT_RST_PIN  GPIO_NUM_21

#define LV_HOR_RES_MAX 240
#define LV_VER_RES_MAX 320

static esp_lcd_panel_handle_t panel_handle = NULL;

static void flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);
static void lv_tick_task(void *arg);
static lv_obj_t *adc_label = NULL;
static char adc_str[32];

void init_display(void) {
    // SPI bus configuration
    spi_bus_config_t buscfg = {
        .mosi_io_num = TFT_MOSI_PIN,
        .sclk_io_num = TFT_SCLK_PIN,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LV_HOR_RES_MAX * 40 * 2
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = TFT_DC_PIN,
        .cs_gpio_num = TFT_CS_PIN,
        .pclk_hz = 10 * 1000 * 1000,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
    };

    esp_lcd_panel_io_handle_t io_handle;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi(SPI2_HOST, &io_config, &io_handle));

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = TFT_RST_PIN,
        .rgb_endian = ESP_LCD_COLOR_SPACE_RGB,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    
    // Add a small delay after init
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // Turn on display
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
    
    // Set display orientation and inversion if needed
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, 0, 0));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, false));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, false));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));

    // Initialize LVGL
    lv_init();

    // Allocate draw buffers for better performance
    buf1 = heap_caps_malloc(LV_HOR_RES_MAX * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);
    buf2 = heap_caps_malloc(LV_HOR_RES_MAX * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);

    // Initialize LVGL draw buffers
    static lv_disp_draw_buf_t draw_buf;
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LV_HOR_RES_MAX * 20);

    // Register display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = flush_cb;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.hor_res = LV_HOR_RES_MAX;
    disp_drv.ver_res = LV_VER_RES_MAX;
    lv_disp_drv_register(&disp_drv);

    // Create and start periodic timer for LVGL tick
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 5000)); // 5ms period
}

static void flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map);
    lv_disp_flush_ready(drv);
}

static void update_display_task(void *pvParameters) {
    TickType_t last_wake_time = xTaskGetTickCount();
    
    while (1) {
        uint32_t adc_value = adc_get_latest_value();
        snprintf(adc_str, sizeof(adc_str), "%lu", adc_value);
        
        if (adc_label != NULL) {
            lv_label_set_text(adc_label, adc_str);
        }
        
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(20));
    }
}

static void lv_tick_task(void *arg) {
    (void) arg;
    lv_tick_inc(1);
}

static void lvgl_handler_task(void *pvParameters) {
    const TickType_t xFrequency = pdMS_TO_TICKS(15);
    TickType_t xLastWakeTime = xTaskGetTickCount();
    
    while (1) {
        lv_timer_handler();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Starting Application");

    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize ADC and start tasks (includes calibration)
    ESP_ERROR_CHECK(adc_init());
    adc_start_task();

    // Wait for ADC calibration to complete before continuing
    while (!adc_is_calibrated()) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    // Initialize the SPP client demo
    spp_client_demo_init();

    ESP_LOGI(TAG, "Initialization complete");

    init_display();
    
    // Create label for ADC value with larger font
    adc_label = lv_label_create(lv_scr_act());
    lv_obj_align(adc_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(adc_label, &lv_font_montserrat_14, 0);  // Use large 48px font
    lv_label_set_text(adc_label, "0");
    
    // Set screen background color to black
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_make(0, 0, 0), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, LV_PART_MAIN);
    
    // Set text color to white
    lv_obj_set_style_text_color(adc_label, lv_color_make(255, 255, 255), LV_PART_MAIN);

    // Create LVGL handler task
    xTaskCreate(lvgl_handler_task, "lvgl_handler", 4096, NULL, 5, NULL);
    
    // Create display update task
    xTaskCreate(update_display_task, "update_display", 4096, NULL, 4, NULL);

    // Main task can now sleep
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

