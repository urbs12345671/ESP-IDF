#pragma once

#define LCD_H_RES          240
#define LCD_V_RES          320
#define LCD_BITS_PIXEL     16  // Recommended value 18 does not work on ESP32 Cheap Yellow Display. There seems to be an error in https://components.espressif.com/components/atanisoft/esp_lcd_ili9488/versions/1.0.9?language=en
#define LCD_BUF_LINES      30
#define LCD_DOUBLE_BUFFER  1
#define LCD_DRAWBUF_SIZE   (LCD_H_RES * LCD_BUF_LINES)
#define LCD_MIRROR_X       (true)  // Recommended value false does not work on ESP32 Cheap Yellow Display. There seems to be an error in https://components.espressif.com/components/espressif/esp_lvgl_port
#define LCD_MIRROR_Y       (false)

#define LCD_PIXEL_CLOCK_HZ (40 * 1000 * 1000)
#define LCD_CMD_BITS       (8)
#define LCD_PARAM_BITS     (8)
#define LCD_SPI_HOST       SPI2_HOST
#define LCD_SPI_CLK        (gpio_num_t) GPIO_NUM_14
#define LCD_SPI_MOSI       (gpio_num_t) GPIO_NUM_13
#define LCD_SPI_MISO       (gpio_num_t) GPIO_NUM_12
#define LCD_DC             (gpio_num_t) GPIO_NUM_2
#define LCD_CS             (gpio_num_t) GPIO_NUM_15
#define LCD_RESET          (gpio_num_t) GPIO_NUM_4
#define LCD_BUSY           (gpio_num_t) GPIO_NUM_NC

#define LCD_BACKLIGHT          (gpio_num_t) GPIO_NUM_21
#define LCD_BACKLIGHT_LEDC_CH  (ledc_channel_t) LEDC_CHANNEL_1

#define TOUCH_CLOCK_HZ  ESP_LCD_TOUCH_SPI_CLOCK_HZ
#define TOUCH_SPI_HOST  SPI3_HOST
#define TOUCH_SPI_CLK   (gpio_num_t) GPIO_NUM_25
#define TOUCH_SPI_MOSI  (gpio_num_t) GPIO_NUM_32
#define TOUCH_SPI_MISO  (gpio_num_t) GPIO_NUM_39
#define TOUCH_CS        (gpio_num_t) GPIO_NUM_33
#define TOUCH_DC        (gpio_num_t) GPIO_NUM_NC
#define TOUCH_RST       (gpio_num_t) GPIO_NUM_NC
#define TOUCH_IRQ       (gpio_num_t) GPIO_NUM_NC  // See an error in monitor when TOUCH_IRQ is set to the recommended value GPIO_NUM_36 and it also prevent even from firing.  

#define TOUCH_X_RES_MIN  0
#define TOUCH_X_RES_MAX  240
#define TOUCH_Y_RES_MIN  0
#define TOUCH_Y_RES_MAX  320