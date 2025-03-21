idf_component_register(
    SRCS "hc_sr04.c" "lvgl_van_ui.c" "touch.c" "lcd.c" "main.c" "lvgl_demo_ui.c"
    INCLUDE_DIRS "."
    REQUIRES "driver")