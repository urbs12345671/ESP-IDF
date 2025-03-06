#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/ledc.h"
#include "ledc_buzzer.h"

#define TAG "ledc_buzzer"

typedef struct {
    const char* name;
    uint32_t frequency;
} note_freq_t;

static const note_freq_t note_table[] = {
    {"C4", 262}, {"D4", 294}, {"E4", 330}, {"F4", 349}, {"G4", 392}, {"A4", 440}, {"B4", 494}, 
    {"C5", 523}, {"D5", 587}, {"E5", 659}, {"F5", 698}, {"G5", 784}, {"A5", 880}, {"B5", 988}, 
    {"C6", 1047}, 
    {"R", 0},
};

static uint32_t get_note_frequency(const char *note) {
    for (int i = 0; i < sizeof(note_table)/sizeof(note_table[0]); i++) {
        if (strcmp(note_table[i].name, note) == 0) {
            return note_table[i].frequency;
        }
    }
    return 0;
}

buzzer_err_t buzzer_init(void) {
    ESP_LOGI(TAG, "Initialize ledc buzzer...");

    // Timer configuration
    ledc_timer_config_t timer_cfg = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = 440,  // Default frequency
        .clk_cfg = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_cfg));

    // Channel configuration
    ledc_channel_config_t channel_cfg = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = BUZZER_GPIO,
        .duty = 0,
        .hpoint = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel_cfg));

    return BUZZER_OK;
}

void buzzer_play_note(const char *note, uint32_t duration_ms) {
    uint32_t freq = get_note_frequency(note);
    
    if (freq == 0 && strcmp(note, "R") != 0) {
        ESP_LOGE(TAG, "Invalid note: %s", note);
        return;
    }

    if (freq > 0) {
        ledc_set_freq(LEDC_MODE, LEDC_TIMER, freq);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, DEFAULT_DUTY);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
    }
    
    vTaskDelay(pdMS_TO_TICKS(duration_ms));
    
    // Automatic silence between notes
    buzzer_stop();
    vTaskDelay(pdMS_TO_TICKS(20));
}

void buzzer_stop(void) {
    ESP_LOGI(TAG, "Stop ledc buzzer...");

    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

void buzzer_deinit(void) {
    buzzer_stop();
    ledc_stop(LEDC_MODE, LEDC_CHANNEL, 0);
    
    ESP_LOGI(TAG, "ledc buzzer de-initialized");
}