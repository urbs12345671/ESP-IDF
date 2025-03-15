#pragma once

#include "esp_err.h"
#include "lvgl.h"

esp_err_t lcd_brightness_init(void);
esp_err_t lcd_brightness_set(int );
esp_err_t lcd_backlight_off(void);
esp_err_t lcd_backlight_on(void);
esp_err_t lcd_rotate(lv_display_t *, lv_display_rotation_t);

esp_err_t lcd_init(esp_lcd_panel_io_handle_t *lcd_panel_io, esp_lcd_panel_handle_t *lcd_panel);
lv_display_t *lvgl_display_init(esp_lcd_panel_io_handle_t lcd_panel_io, esp_lcd_panel_handle_t lcd_panel);