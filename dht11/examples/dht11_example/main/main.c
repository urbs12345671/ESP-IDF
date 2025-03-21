#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
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
#include "driver/gpio.h"
#include "lcd.h"
#include "touch.h"
#include "lvgl_van_ui.h"
#include "job.h"
#include "dht11.h"

#define DHT11_GPIO_PIN GPIO_NUM_27

QueueHandle_t result_queue = NULL;
QueueHandle_t job_queue = NULL;

static const char *TAG = "main";

static void worker_task(void *arg) {
    job_t job;
    job_result_t job_result;

    while (1) {
        // Wait for incoming job commands
        if (xQueueReceive(job_queue, &job, portMAX_DELAY)) {
            //ESP_LOGI(TAG, "Job received (id = %d)", cmd.job_id);

            float temperature = 0.0, humidity = 0.0;
            if (dht11_read(&temperature, &humidity)) {
                ESP_LOGI(TAG, "Temperature->%.1f C  Humidity->%.1f%%", temperature, humidity);

                // Send progress updates
                job_result.job_id = job.job_id;
                job_result.progress = 100;
                job_result.completed = 100;
                job_result.temperature = temperature;
                job_result.humidity = humidity;

                xQueueSend(result_queue, &job_result, 0);                
            }

            // // Simulate long-running process
            // for (int progress = 0; progress <= 100; progress += 10) {
            //     vTaskDelay(pdMS_TO_TICKS(500));  // Simulate work

            //     // Send progress updates
            //     res.job_id = cmd.job_id;
            //     res.progress = progress;
            //     res.completed = (progress == 100);
            //     xQueueSend(result_queue, &res, 0);
            // }
        }
    }
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());

    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "APP is start...\n");
    ESP_LOGI(TAG, "IDF Version: %d.%d.%d", ESP_IDF_VERSION_MAJOR, ESP_IDF_VERSION_MINOR, ESP_IDF_VERSION_PATCH);
    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "Free memory: %lu bytes", esp_get_free_heap_size());

    dht11_init(DHT11_GPIO_PIN);

    // Create queues
    job_queue = xQueueCreate(5, sizeof(job_t));
    result_queue = xQueueCreate(5, sizeof(job_result_t));

    // Create worker task
    xTaskCreate(worker_task, "worker_task", 4096, NULL, 2, NULL);
    
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
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "lvgl_port_init() failed: %s", esp_err_to_name(err));
    } 

    // Initialize lvgl display
    lv_display_t *lvgl_display = lvgl_display_init(lcd_panel_io, lcd_panel);
    if (lvgl_display == NULL) {
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

    // Rotate the display to landscape mode
    ESP_ERROR_CHECK(lcd_rotate(lvgl_display, LV_DISPLAY_ROTATION_270));

    // Add lvgl_demo_ui to lvgl display
    lvgl_van_ui(lvgl_display);

    // Adjust lcd brightness to make it visible
    ESP_ERROR_CHECK(lcd_brightness_set(100));

    while(1) {
        vTaskDelay(pdMS_TO_TICKS(50));    
    }
}
