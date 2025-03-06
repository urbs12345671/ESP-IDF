#include <stdio.h>
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define TAG "main"
#define CORE0  0
// only define xCoreID CORE1 as 1 if this is a multiple core processor target, else define it as tskNO_AFFINITY
#define CORE1  ((CONFIG_FREERTOS_NUMBER_OF_CORES > 1) ? 1 : tskNO_AFFINITY)
#define GPIO_PIN GPIO_NUM_25

static int iteration = 0;
static TaskHandle_t taskHandle;

void task(void* param)
{
    ESP_LOGI(TAG, "Task started...");
   
    printf("Hello world!\n");
    
    int gpio_level = 0;
    while (1)
    {
        ESP_LOGI(TAG, "iteration = %i", iteration);
        if (iteration <= 20)
        {
            gpio_level = gpio_level ? 0: 1;
            gpio_set_level(GPIO_PIN, gpio_level);
            ESP_LOGI(TAG, "GPIO level = %i", gpio_level);            
            vTaskDelay(pdMS_TO_TICKS(1000));

            ++iteration;
        }
        else
        {
            // Ensure the GPIO level is set to low before task is ended
            gpio_set_level(GPIO_PIN, 0);
            // Flush the stdout before task is ended
            fflush(stdout);
            vTaskDelete(taskHandle);
        }
    }    
}

void app_main(void)
{
    /* Print ESP32 chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("%s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");
    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if (esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }
    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    gpio_config_t config = {
        .pin_bit_mask = (1<<GPIO_PIN),
        .mode = GPIO_MODE_OUTPUT,        
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&config);

    xTaskCreatePinnedToCore(task, "Task", 2048, NULL, 3, &taskHandle, CORE1);
}
