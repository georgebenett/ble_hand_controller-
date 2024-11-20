idf_component_register(
    SRCS "main.c" "spp_client_demo.c" "adc.c"
    INCLUDE_DIRS "."
    REQUIRES driver 
            nvs_flash 
            bt
            esp_adc
)
