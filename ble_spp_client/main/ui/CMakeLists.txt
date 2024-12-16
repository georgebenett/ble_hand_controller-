SET(SOURCES screens/ui_home_screen.c
    screens/ui_shutdown_screen.c
    screens/ui_detailed_home.c
    screens/ui_splash_screen.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    images/ui_img_battery_icon_png.c
    images/ui_img_no_connection_png.c
    images/ui_img_gb_png.c
    fonts/ui_font_bebas120.c
    fonts/ui_font_bebas25.c
    fonts/ui_font_bebas30.c
    fonts/ui_font_bebas_14.c
    fonts/ui_font_bebas_medium.c
    fonts/ui_font_bebas_small.c)

add_library(ui ${SOURCES})
