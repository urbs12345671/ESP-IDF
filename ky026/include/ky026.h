#pragma once
#include <stdbool.h>
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief KY-026 Flame Sensor Configuration Structure
 * 
 * @note ADC1 Channel Restrictions:
 * - Avoid ADC1_CH0 (GPIO36) and ADC1_CH3 (GPIO39) when using Wi-Fi/BT
 * - For Wi-Fi/BT applications, prefer ADC2 channels with proper locking
 *
 * - Recommended ADC attenuation for ESP-IDF v5.4+
 * - Use ADC_ATTEN_DB_12 instead of ADC_ATTEN_DB_11
 * - DB_12 provides better linearity and calibration support
*/
typedef struct {
    gpio_num_t digital_pin;        // GPIO pin for digital output
    bool digital_active_low;       // True if digital signal is active low
    adc_unit_t adc_unit;           // ADC unit (ADC_UNIT_1 or ADC_UNIT_2)
    adc_channel_t analog_channel;  // ADC channel (e.g., ADC_CHANNEL_0)
    adc_atten_t analog_atten;      // ADC attenuation (e.g., ADC_ATTEN_DB_12)
    adc_bitwidth_t bit_width;      // ADC bit width (e.g., ADC_BITWIDTH_12)
    int analog_threshold;          // Software threshold for analog detection
} ky026_config_t;

typedef void *ky026_handle_t;

/**
 * @brief Initialize the KY-026 flame sensor
 * @param config Configuration parameters
 * @return Handle to the KY-026 sensor, or NULL on failure
 */
ky026_handle_t ky026_init(const ky026_config_t *config);

/**
 * @brief Read the digital output (flame detected)
 * @param handle KY-026 handle
 * @return True if flame is detected based on digital output
 */
bool ky026_read_digital(ky026_handle_t handle);

/**
 * @brief Read the raw analog value
 * @param handle KY-026 handle
 * @return Raw ADC value (0-4095 for 12-bit)
 */
int ky026_read_analog_raw(ky026_handle_t handle);

/**
 * @brief Check if analog value exceeds the threshold
 * @param handle KY-026 handle
 * @return True if analog value > threshold
 */
bool ky026_analog_above_threshold(ky026_handle_t handle);

/**
 * @brief Set a new analog threshold
 * @param handle KY-026 handle
 * @param threshold New threshold value
 */
void ky026_set_analog_threshold(ky026_handle_t handle, int threshold);

/**
 * @brief Per ESP-IDF v5.4 document, ADC2 requires locking when used with Wi-Fi
 * @param handle KY-026 handle
 */
void ky026_adc2_lock(ky026_handle_t handle);

/**
 * @brief Per ESP-IDF v5.4 document, ADC2 requires locking when used with Wi-Fi. This function unlock the ADC2 after use. 
 * @param handle KY-026 handle
 */
void ky026_adc2_unlock(ky026_handle_t handle);


#ifdef __cplusplus
}
#endif