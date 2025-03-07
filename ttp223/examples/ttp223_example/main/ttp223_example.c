#include "ttp223.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "TTP223_EXAMPLE";

// Add function prototype declaration first
static void ttp223_task(void *pvParameters);

void app_main()
{
    // Configure TTP223
    ttp223_config_t config = {
        .gpio_num = GPIO_NUM_25,
        .pull_up_en = false,
        .pull_down_en = false};

    // Initialize touch sensor
    ESP_ERROR_CHECK(ttp223_init(&config));

    // Install GPIO ISR service
    gpio_install_isr_service(0);

    // Register interrupt handler
    ESP_ERROR_CHECK(ttp223_register_isr_handler(GPIO_NUM_25));

    // Create processing task
    xTaskCreate(ttp223_task, "ttp223_task", 2048, NULL, 5, NULL);

    // Main loop
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Now define the task function
static void ttp223_task(void *pvParameters)
{
    touch_event_t evt;
    while (1)
    {
        if (ttp223_get_event(&evt, portMAX_DELAY) == ESP_OK)
        {
            ESP_LOGI(TAG, "Touch event on GPIO %d: %s",
                     evt.gpio_num,
                     evt.touched ? "TOUCHED" : "RELEASED");
        }
    }
}