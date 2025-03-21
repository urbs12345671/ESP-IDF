#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "esp_err.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "lvgl_van_ui.h"
#include "job.h"

LV_IMG_DECLARE(temperature_16)
LV_IMG_DECLARE(humidity_16)

extern QueueHandle_t result_queue;
extern QueueHandle_t job_queue;

static const char *TAG = "lvgl_van_ui";

static lv_obj_t *temperature_label;
static lv_obj_t *humidity_label;
static lv_timer_t *lv_timer;

static void lv_timer_cb(struct _lv_timer_t *timer) {
    lvgl_port_lock(0);

    //ESP_LOGI(TAG, "inside lv_timer_cb");

    job_result_t job_result;
    char buf[32];

    // Check for results without blocking
    while (xQueueReceive(result_queue, &job_result, 0) == pdTRUE) {
        ESP_LOGI(TAG, "Job result received: Temperature = %.1f C    Humidity=%.1f%%", job_result.temperature, job_result.humidity);
        if (job_result.completed) {
            snprintf(buf, sizeof(buf), "%.1f C", job_result.temperature);  
            lv_label_set_text(temperature_label, buf);
            snprintf(buf, sizeof(buf), "%.1f%%", job_result.humidity);  
            lv_label_set_text(humidity_label, buf);            
        // } else {
        //     snprintf(buf, sizeof(buf), "Progress: %d%%", job_result.progress);
        //     lv_label_set_text(sensor_label, buf);
        }
    } 

    vTaskDelay(pdMS_TO_TICKS(2000));  

    static int job_counter = 0;
    job_t job = {
        .job_id = job_counter++
    };

    // Send command to worker task to request update
    if (xQueueSend(job_queue, &job, pdMS_TO_TICKS(100)) != pdPASS) {
        // Handle queue full error
        ESP_LOGI(TAG, "System busy...");
    // } else {
    //     ESP_LOGI(TAG, "Job queued (id = %d)", job.job_id);
    }

    lvgl_port_unlock();
}

esp_err_t lvgl_van_ui(lv_display_t *disp) {
    /* Wait for the other task done the screen operation */
    lvgl_port_lock(0);

    lv_obj_t *parent = lv_display_get_screen_active(disp);

    lv_obj_t *temperature_image = lv_img_create(parent);
    lv_img_set_src(temperature_image, &temperature_16);
    lv_obj_set_pos(temperature_image, 40, 40);

    lv_obj_t *humidity_image = lv_img_create(parent);
    lv_img_set_src(humidity_image, &humidity_16);
    lv_obj_set_pos(humidity_image, 40, 100);   

    temperature_label = lv_label_create(parent);
    lv_label_set_text(temperature_label, "temperature");
    lv_obj_align(temperature_label, LV_ALIGN_TOP_LEFT, 80, 40);

    humidity_label = lv_label_create(parent);
    lv_label_set_text(humidity_label, "humidity");
    lv_obj_align(humidity_label, LV_ALIGN_TOP_LEFT, 80, 100);

    lv_timer = lv_timer_create(lv_timer_cb, 5, NULL);

    lvgl_port_unlock();

    return ESP_OK;;
}