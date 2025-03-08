#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ky023.h"

void app_main(void) {
    ky023_config_t config = {
        .x_gpio = GPIO_NUM_32,
        .y_gpio = GPIO_NUM_33,
        .button_gpio = GPIO_NUM_25,
        .attenuation = ADC_ATTEN_DB_12
    };

    ESP_ERROR_CHECK(ky023_init(&config));

    while(1) {
        uint16_t x = ky023_read_x();
        uint16_t y = ky023_read_y();
        bool btn = ky023_read_button();

        printf("X: %4d | Y: %4d | Button: %d\n", x, y, btn);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    ky023_cleanup();
}