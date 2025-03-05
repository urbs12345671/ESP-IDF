#include <stdbool.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <esp_log.h>
#include "dht11.h"

#define TAG "main"
#define DHT11_GPIO_PIN 25 // DHT11 GPIO pin

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "APP is start...\n");
    ESP_LOGI(TAG, "IDF Version: %d.%d.%d", ESP_IDF_VERSION_MAJOR, ESP_IDF_VERSION_MINOR, ESP_IDF_VERSION_PATCH);
    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "Free memory: %lu bytes", esp_get_free_heap_size());

    float temperature = 0, humidity = 0;

    dht11_init(DHT11_GPIO_PIN);

    while (1)
    {
        if (dht11_read(&temperature, &humidity))
        {
            ESP_LOGI(TAG, "Temperature->%.1f C     Humidity->%.1f%%", temperature, humidity);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
