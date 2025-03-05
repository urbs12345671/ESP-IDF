#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "hc_sr04.h"

#define TAG "main"
#define GPIO_NUM_32 32
#define GPIO_NUM_33 33

void app_main(void)
{
    hc_sr04_t sensor = {
        .trig_gpio = GPIO_NUM_32,
        .echo_gpio = GPIO_NUM_33,
    };

    if (!hc_sr04_init(&sensor))
    {
        ESP_LOGE(TAG, "Failed to initialize HC-SR04");
        return;
    }

    while (1)
    {
        float distance;
        if (hc_sr04_measure_distance(&sensor, &distance, 1000))
        {
            ESP_LOGI(TAG, "Distance: %.2f cm", distance);
        }
        else
        {
            ESP_LOGW(TAG, "Measurement failed or timeout");
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}