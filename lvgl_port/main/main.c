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
#include "lvgl_van_ui.h"
#include "driver/gpio.h"
#include "ky026.h"
#include "job.h"


QueueHandle_t result_queue = NULL;
QueueHandle_t job_queue = NULL;

static const char *TAG = "main";

static ky026_handle_t sensor = NULL;

static void worker_task(void *arg) {
    job_command_t cmd;
    job_result_t res;

    while (1) {
        // Wait for incoming job commands
        if (xQueueReceive(job_queue, &cmd, portMAX_DELAY)) {

            //ESP_LOGI(TAG, "Job received (id = %d)", cmd.job_id);

            bool digital_state = ky026_read_digital(sensor);
            //ESP_LOGI(TAG, "digital_state: %d", digital_state);

            if (digital_state)
            {
                printf("Flame: %s", digital_state ?  "DETECTED" : "CLEARED"); // digital_active_low = true
        
                // Send progress updates
                res.job_id = cmd.job_id;
                res.progress = 100;
                res.completed = 100;
                res.sensor_digital_result = digital_state;

                xQueueSend(result_queue, &res, 0);
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

void app_main(void)
{
    ky026_config_t config = {
        .digital_pin = GPIO_NUM_22,
        .digital_active_low = true,
        .adc_unit = ADC_UNIT_2,           // Safer choice with Wi-Fi
        .analog_channel = ADC_CHANNEL_7,  // GPIO27 (ADC1_CH6)
        .analog_atten = ADC_ATTEN_DB_12,  // Use DB_12 per v5.4 documentation 
        .bit_width = ADC_BITWIDTH_12,
        .analog_threshold = 2000,
    };  

    sensor = ky026_init(&config);
    if (!sensor) {
        ESP_LOGE(TAG, "Flame sensor initialization failed");
    }

    // Create queues
    job_queue = xQueueCreate(5, sizeof(job_command_t));
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

    // Rotate the display to landscape mode
    ESP_ERROR_CHECK(lcd_rotate(lvgl_display, LV_DISPLAY_ROTATION_0));

    // Add lvgl_demo_ui to lvgl display
    lvgl_van_ui(lvgl_display);

    // Adjust lcd brightness to make it visible
    ESP_ERROR_CHECK(lcd_brightness_set(100));

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(50));    
    }
}
