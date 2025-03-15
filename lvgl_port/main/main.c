//#include <sys/lock.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_lcd_ili9341.h"
#include "esp_lcd_touch_xpt2046.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "lcd.h"
#include "touch.h"
#include "lvgl_demo_ui.h"

static const char *TAG = "main";

void app_main(void)
{
    // Initialize the ledc channel which controls the brightness of the lcd
    ESP_ERROR_CHECK(lcd_brightness_init());

    // Initialize the ili9341 lcd to use spi 2
    esp_lcd_panel_io_handle_t lcd_panel_io;
    esp_lcd_panel_handle_t lcd_panel;
    ESP_ERROR_CHECK(lcd_init(&lcd_panel_io, &lcd_panel));

    // Initialize lvgl port
    const lvgl_port_cfg_t lvgl_port_cfg = {
        .task_priority = 4,
        .task_stack = 4096,
        .task_affinity = -1,
        .task_max_sleep_ms = 500,
        .timer_period_ms = 5
    };
    esp_err_t err = lvgl_port_init(&lvgl_port_cfg);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "lvgl_port_init() failed: %s", esp_err_to_name(err));
    } 

    // Initialize lvgl display
    lv_display_t *lvgl_display = lvgl_display_init(lcd_panel_io, lcd_panel);
    if (lvgl_display == NULL)
    {
        ESP_LOGI(TAG, "fatal error in lvgl_display_init");
        esp_restart();
    }
    
    // Initialize xpt2046 touch device to use spi 3
    esp_lcd_touch_handle_t lcd_touch;    
    ESP_ERROR_CHECK(touch_init(&lcd_touch));

    // Add touch input to lvgl port
    const lvgl_port_touch_cfg_t lvgl_port_touch_cfg = {
        .disp = lvgl_display,
        .handle = lcd_touch,
    };
    lvgl_port_add_touch(&lvgl_port_touch_cfg);

    // Add lvgl_demo_ui to lvgl display
    lvgl_demo_ui(lvgl_display);

    // Adjust lcd brightness to make it visible
    ESP_ERROR_CHECK(lcd_brightness_set(100));
    // Rotate the display to landscape mode
    ESP_ERROR_CHECK(lcd_rotate(lvgl_display, LV_DISPLAY_ROTATION_0));

    while(1)
    {
        //ESP_ERROR_CHECK(esp_lcd_touch_read_data(lcd_touch));
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
