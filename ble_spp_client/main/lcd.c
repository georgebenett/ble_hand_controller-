#include "lcd.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ui/ui.h"
#include "adc.h"
#include "ble_spp_client.h"
#include "vesc_config.h"
#include "ui_updater.h"

// Static variables
static esp_lcd_panel_handle_t panel_handle = NULL;
static lv_color_t *buf1 = NULL;
static lv_color_t *buf2 = NULL;
static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;
static esp_timer_handle_t periodic_timer;


// Function prototypes
static void flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map);
static void lv_tick_task(void *arg);
static void lvgl_handler_task(void *pvParameters);
static void display_update_task(void *pvParameters);

void lcd_init(void) {
    // Configure GPIO20 and GPIO9
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << TFT_GND_PIN) | (1ULL << TFT_VDD_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    //power on the display
    ESP_ERROR_CHECK(gpio_set_level(TFT_GND_PIN, 0));
    ESP_ERROR_CHECK(gpio_set_level(TFT_VDD_PIN, 1));

    spi_bus_config_t buscfg = {
        .mosi_io_num = TFT_MOSI_PIN,
        .sclk_io_num = TFT_SCLK_PIN,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SOC_SPI_MAXIMUM_BUFFER_SIZE
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = TFT_DC_PIN,
        .cs_gpio_num = TFT_CS_PIN,
        .pclk_hz = 80 * 1000 * 1000,
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
    vTaskDelay(pdMS_TO_TICKS(100));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, 0, 0));
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, false));
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, false));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));

    lv_init();

    // Allocate two buffers for double buffering with 1/X screen size
    buf1 = heap_caps_malloc(LV_HOR_RES_MAX * (LV_VER_RES_MAX/16) * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);
    buf2 = heap_caps_malloc(LV_HOR_RES_MAX * (LV_VER_RES_MAX/16) * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2 != NULL);

    // Initialize with both buffers
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LV_HOR_RES_MAX * (LV_VER_RES_MAX/16));

    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = flush_cb;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.hor_res = LV_HOR_RES_MAX;
    disp_drv.ver_res = LV_VER_RES_MAX;
    disp_drv.physical_hor_res = LV_HOR_RES_MAX;
    disp_drv.physical_ver_res = LV_VER_RES_MAX;
    disp_drv.offset_x = 0;
    disp_drv.offset_y = 0;
    lv_disp_drv_register(&disp_drv);

    // Clear the screen to black at initialization
    //ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, 0, 0, LV_HOR_RES_MAX, LV_VER_RES_MAX, NULL));

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"
    };
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1000));


    // Start display tasks
    lcd_start_tasks();
}

static void flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map);
    lv_disp_flush_ready(drv);
}

static void lv_tick_task(void *arg) {
    (void) arg;
    lv_tick_inc(1);
}

static void lvgl_handler_task(void *pvParameters) {
    const TickType_t xFrequency = pdMS_TO_TICKS(10);
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) {
        lv_timer_handler();
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

static void display_update_task(void *pvParameters) {
    vesc_config_t config;
    ESP_ERROR_CHECK(vesc_config_load(&config));

    ui_updater_init();

    while (1) {
        // Get latest values
        int32_t speed = vesc_config_get_speed(&config);
        ui_update_speed(speed);

        // Update other values as needed
        // ui_update_battery_voltage(...);
        // ui_update_motor_current(...);
        // etc.

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void lcd_start_tasks(void) {
    xTaskCreate(lvgl_handler_task, "lvgl_handler", 4096, NULL, 5, NULL);
    xTaskCreate(display_update_task, "display_update", 2048, NULL, 5, NULL);
}

