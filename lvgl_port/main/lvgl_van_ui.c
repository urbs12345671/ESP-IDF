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

extern QueueHandle_t result_queue;
extern QueueHandle_t job_queue;

static const char *TAG = "lvgl van ui";

static lv_obj_t *sensor_label;
static lv_timer_t *lv_timer;

static void lv_timer_cb(struct _lv_timer_t *t)
{
    lvgl_port_lock(0);

    //ESP_LOGI(TAG, "inside lv_timer_cb");
    // Assume sensor is not triggered
    bool sensor_triggered = false;

    job_result_t res;
    char buf[32];

    // Check for results without blocking
    while (xQueueReceive(result_queue, &res, 0) == pdTRUE) {

        ESP_LOGI(TAG, "Result received: %d", res.sensor_digital_result);
 
        if (res.completed) {
            snprintf(buf, sizeof(buf), "%s", (res.sensor_digital_result ?  "DETECTED" : "CLEARED"));  
            sensor_triggered =  (res.sensor_digital_result ?  true : false);  
            lv_label_set_text(sensor_label, buf);
        } else {
            snprintf(buf, sizeof(buf), "Progress: %d%%", res.progress);
            lv_label_set_text(sensor_label, buf);
        }
    } 
    
    if (!sensor_triggered)
    {
        snprintf(buf, sizeof(buf), "%s", "CLEARED");  
        lv_label_set_text(sensor_label, buf);        
    }

    static int job_counter = 0;
    job_command_t cmd = {
        .job_id = job_counter++,
        // Add other parameters
    };

    // Send command to worker task (non-blocking)
    if (xQueueSend(job_queue, &cmd, pdMS_TO_TICKS(100)) != pdPASS) {
        // Handle queue full error
        ESP_LOGI(TAG, "System busy!");
    }
    // else
    // {
    //     ESP_LOGI(TAG, "job queued (id = %d)", cmd.job_id);
    // }

    lvgl_port_unlock();
}

esp_err_t lvgl_van_ui(lv_display_t *disp)
{
    /* Wait for the other task done the screen operation */
    lvgl_port_lock(0);

    lv_obj_t *scr = lv_display_get_screen_active(disp);

    sensor_label = lv_label_create(scr);
    lv_label_set_text(sensor_label, "Sensor is ready...");
    lv_obj_align(sensor_label, LV_ALIGN_TOP_MID, 0, 10);

    lv_timer = lv_timer_create(lv_timer_cb, 5, NULL);

    lvgl_port_unlock();

    return ESP_OK;;
}