#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_lcd_panel_io.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/spi_master.h"
#include "esp_lcd_ili9341.h"
#include "esp_lvgl_port.h"
#include "hardware.h"

static const char *TAG="lcd";

esp_err_t lcd_brightness_init(void)
{
    const ledc_channel_config_t lcd_channel_config = {
        .gpio_num = LCD_BACKLIGHT,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LCD_BACKLIGHT_LEDC_CH,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = 1,
        .duty = 0,
        .hpoint = 0,
        .flags.output_invert = false
    };
    ESP_ERROR_CHECK(ledc_channel_config(&lcd_channel_config));
 
    const ledc_timer_config_t lcd_timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = 1,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&lcd_timer_config));

    return ESP_OK;
}

esp_err_t lcd_brightness_set(int brightness_percent)
{
    if (brightness_percent > 100) {
        brightness_percent = 100;
    }
    if (brightness_percent < 0) {
        brightness_percent = 0;
    }
    ESP_LOGI(TAG, "set lcd backlight: %d%%", brightness_percent);

    uint32_t duty_cycle = (1023 * brightness_percent) / 100;

    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LCD_BACKLIGHT_LEDC_CH, duty_cycle));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LCD_BACKLIGHT_LEDC_CH));

    return ESP_OK;
}

esp_err_t lcd_backlight_off(void)
{
    return lcd_brightness_set(0);
}

esp_err_t lcd_backlight_on(void)
{
    return lcd_brightness_set(100);
}

esp_err_t lcd_rotate(lv_display_t *lvgl_disp, lv_display_rotation_t dir)
{
    if (lvgl_disp)
    {
        lv_display_set_rotation(lvgl_disp, dir);
        return ESP_OK;
    }

    return ESP_FAIL;
}

esp_err_t lcd_init(esp_lcd_panel_io_handle_t *lcd_panel_io, esp_lcd_panel_handle_t *lcd_panel)
{
    const spi_bus_config_t buscfg = { 
        .mosi_io_num = LCD_SPI_MOSI,
        .miso_io_num = LCD_SPI_MISO,
        .sclk_io_num = LCD_SPI_CLK,
        .quadhd_io_num = GPIO_NUM_NC,
        .quadwp_io_num = GPIO_NUM_NC,
        .max_transfer_sz = LCD_DRAWBUF_SIZE * sizeof(uint16_t),
    };

    ESP_RETURN_ON_ERROR(spi_bus_initialize(LCD_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO), TAG, "spi init failed");

    const esp_lcd_panel_io_spi_config_t io_config = {
        .cs_gpio_num = LCD_CS,
        .dc_gpio_num = LCD_DC,
        .spi_mode = 0,
        .pclk_hz = LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
    };
    // Per https://components.espressif.com/components/espressif/esp_lvgl_port
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_SPI_HOST, &io_config, lcd_panel_io), TAG, "LCD new panel io failed");


    const esp_lcd_panel_dev_config_t lcd_panel_dev_config = {
        .reset_gpio_num = LCD_RESET,  // Per https://components.espressif.com/components/atanisoft/esp_lcd_ili9488/versions/1.0.9?language=en
        .color_space = ESP_LCD_COLOR_SPACE_BGR,
        .bits_per_pixel = LCD_BITS_PIXEL,  
    };

    esp_err_t err = esp_lcd_new_panel_ili9341(*lcd_panel_io, &lcd_panel_dev_config, lcd_panel);

    esp_lcd_panel_reset(*lcd_panel);
    esp_lcd_panel_init(*lcd_panel);
    esp_lcd_panel_mirror(*lcd_panel, LCD_MIRROR_X, LCD_MIRROR_Y);
    esp_lcd_panel_disp_on_off(*lcd_panel, true);

    return err;
}

lv_display_t *lvgl_display_init(esp_lcd_panel_io_handle_t lcd_panel_io, esp_lcd_panel_handle_t lcd_panel)
{
    ESP_LOGD(TAG, "add lcd screen");
    const lvgl_port_display_cfg_t lvgl_port_display_cfg = {
        .io_handle = lcd_panel_io,
        .panel_handle = lcd_panel,
        .buffer_size = LCD_DRAWBUF_SIZE,
        .double_buffer = LCD_DOUBLE_BUFFER,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = false,
        .color_format = LV_COLOR_FORMAT_RGB565,
        .rotation = {
            .swap_xy = false,
            .mirror_x = LCD_MIRROR_X,  
            .mirror_y = LCD_MIRROR_Y,
        },
        .flags = {
            .buff_dma = true,
            .buff_spiram = false,  // Per https://components.espressif.com/components/espressif/esp_lvgl_port
            .swap_bytes = true,  // Recommended value (false) does not work on ESP32 Cheap Yellow Display. Per https://components.espressif.com/components/espressif/esp_lvgl_port
            .sw_rotate = true,  // Per https://components.espressif.com/components/espressif/esp_lvgl_port
        }
    };
    
    return lvgl_port_add_disp(&lvgl_port_display_cfg);
}
