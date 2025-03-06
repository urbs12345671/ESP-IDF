#pragma once

#include <stdint.h>
#include "driver/ledc.h"

// Configuration
#define BUZZER_GPIO          GPIO_NUM_25
#define LEDC_TIMER           LEDC_TIMER_0
#define LEDC_MODE            LEDC_HIGH_SPEED_MODE
#define LEDC_CHANNEL         LEDC_CHANNEL_0
#define LEDC_DUTY_RES        LEDC_TIMER_13_BIT
#define DEFAULT_DUTY         (4095)  // 50% for 13-bit resolution

typedef enum {
    BUZZER_OK,
    BUZZER_ERR_INVALID_NOTE,
    BUZZER_ERR_CONFIG
} buzzer_err_t;

// Public functions
buzzer_err_t buzzer_init(void);
void buzzer_play_note(const char *note, uint32_t duration_ms);
void buzzer_stop(void);
void buzzer_deinit(void);