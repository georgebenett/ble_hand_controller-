idf_component_register(
    SRCS "main.c" "spp_client_demo.c" "adc.c" "lcd.c"
    INCLUDE_DIRS "."
    REQUIRES driver 
            nvs_flash 
            bt
            esp_adc
            spi_flash
            esp_lcd
            lvgl
)
