file(GLOB_RECURSE UI_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/ui/*.c"
)

idf_component_register(
    SRCS
        "button.c"
        "sleep.c"
        "ble_spp_client.c"
        "main.c"
        "adc.c"
        "lcd.c"
        "vesc_config.c"
        "ui_updater.c"
        "lcd.c"
        "viber.c"
        "hw_config.c"
        ${UI_SOURCES}
    INCLUDE_DIRS
        "."
        "ui"
    REQUIRES driver nvs_flash bt esp_adc spi_flash esp_lcd lvgl esp_lcd_sh8601 esp_lcd_touch_ft5x06
)
