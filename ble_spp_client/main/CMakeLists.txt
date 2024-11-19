idf_component_register(
    SRCS "main.c" "lcd.c" "spp_client_demo.c" "adc.c"
    INCLUDE_DIRS "."
    REQUIRES driver 
            esp_lcd 
            nvs_flash 
            bt
            esp_adc
)
