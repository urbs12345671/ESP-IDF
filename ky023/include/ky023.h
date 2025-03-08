#ifndef KY023_H
#define KY023_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"

typedef struct {
    gpio_num_t x_gpio;        // GPIO for X axis (ADC1)
    gpio_num_t y_gpio;        // GPIO for Y axis (ADC1)
    gpio_num_t button_gpio;   // GPIO for button input
    adc_atten_t attenuation;  // ADC attenuation (e.g., ADC_ATTEN_DB_11)
} ky023_config_t;

/**
 * @brief Initialize KY-023 joystick component
 * 
 * @param config Configuration structure
 * @return esp_err_t ESP_OK on success
 */
esp_err_t ky023_init(const ky023_config_t *config);

/**
 * @brief Read X-axis value
 * 
 * @return uint16_t Raw ADC value (0-4095)
 */
uint16_t ky023_read_x(void);

/**
 * @brief Read Y-axis value
 * 
 * @return uint16_t Raw ADC value (0-4095)
 */
uint16_t ky023_read_y(void);

/**
 * @brief Read button state
 * 
 * @return true Button pressed
 * @return false Button released
 */
bool ky023_read_button(void);

/**
 * @brief Clean up
 * 
 * @return None
 * @return None
 */
void ky023_cleanup(void);

#endif // KY023_H