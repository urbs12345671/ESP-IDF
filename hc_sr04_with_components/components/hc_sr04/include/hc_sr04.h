#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"
#include "driver/mcpwm_cap.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

typedef struct
{
    gpio_num_t trig_gpio;
    gpio_num_t echo_gpio;
    mcpwm_cap_timer_handle_t cap_timer;
    mcpwm_cap_channel_handle_t cap_channel;
    TaskHandle_t task_handle;
} hc_sr04_t;

/**
 * @brief Initialize HC-SR04 sensor
 * @param config Sensor configuration (GPIOs, timer, etc.)
 * @return true if initialization succeeded, false otherwise
 */
bool hc_sr04_init(hc_sr04_t *config);

/**
 * @brief Measure distance in centimeters
 * @param config Sensor configuration
 * @param distance_out Pointer to store measured distance
 * @param timeout_ms Measurement timeout in milliseconds
 * @return true if measurement succeeded, false otherwise
 */
bool hc_sr04_measure_distance(hc_sr04_t *config, float *distance_out, uint32_t timeout_ms);