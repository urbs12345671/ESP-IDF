#pragma once

#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    adc_unit_t adc_unit;         // ADC unit (ADC_UNIT_1)
    adc_channel_t adc_channel;   // ADC channel (e.g., ADC_CHANNEL_0)
    adc_atten_t adc_atten;       // ADC attenuation (e.g., ADC_ATTEN_DB_12)
    gpio_num_t digital_pin;      // GPIO number for digital output
    bool pull_up_en;             // Enable internal pull-up
    bool pull_down_en;           // Enable internal pull-down
} ky038_config_t;

typedef void (*ky038_isr_callback_t)(void);

typedef struct {
    ky038_config_t config;
    adc_oneshot_unit_handle_t adc_handle;
    ky038_isr_callback_t user_callback;
} ky038_handle_t;

esp_err_t ky038_init(ky038_handle_t *handle, const ky038_config_t *config);
int ky038_read_analog(ky038_handle_t *handle);
int ky038_read_digital(ky038_handle_t *handle);
esp_err_t ky038_register_isr_handler(ky038_handle_t *handle, gpio_int_type_t intr_type, ky038_isr_callback_t callback);
esp_err_t ky038_deinit(ky038_handle_t *handle);

#ifdef __cplusplus
}
#endif