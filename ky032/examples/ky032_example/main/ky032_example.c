#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "ky032.h"

static const char *TAG = "KY032_EXAMPLE";

static QueueHandle_t gpio_evt_queue = NULL;

static void IRAM_ATTR obstacle_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

void monitor_task(void* arg)
{
    uint32_t io_num;
    while(1) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            bool state = ky032_read(io_num);
            ESP_LOGI(TAG, "Obstacle %s on GPIO %" PRIu32, 
                    state ? "cleared" : "detected", io_num);
        }
    }
}

void app_main(void)
{
    gpio_num_t obstacle_pin = GPIO_NUM_5;
    gpio_num_t en_pin = GPIO_NUM_4;
    
    // Initialize sensor
    ESP_ERROR_CHECK(ky032_init(obstacle_pin, en_pin));

    ky032_enable(en_pin); 
    
    // Create event queue and task
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(monitor_task, "monitor_task", 2048, NULL, 10, NULL);
    
    // Install ISR service
    gpio_install_isr_service(0);
    ky032_install_isr(obstacle_pin, GPIO_INTR_NEGEDGE, obstacle_isr_handler, (void*) obstacle_pin);

    // Periodic status check
    while(1) {

        if(ky032_read(obstacle_pin)) {
            ky032_disable(en_pin);  // Disable when obstacle detected
            vTaskDelay(pdMS_TO_TICKS(2000));
            ky032_enable(en_pin);   // Re-enable after 2 seconds
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}