idf_component_register(
    SRCS 
                "button.c" 
                "sleep.c" 
                "ble_spp_client.c" 
                "main.c" 
                "adc.c" 
                "lcd.c"
                "ui/screens/ui_Screen1.c"
                "ui/ui.c"
                "ui/components/ui_comp_hook.c"
                "ui/ui_helpers.c"
                "ui/fonts/ui_font_bebas120.c"
    INCLUDE_DIRS 
                "."
                "ui"
                
    REQUIRES driver 
            nvs_flash 
            bt
            esp_adc
            spi_flash
            esp_lcd
            lvgl
)
