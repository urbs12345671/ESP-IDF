#pragma once
#include <driver/gpio.h>
#include "esp_adc/adc_oneshot.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize KY-036 touch sensor
 * @note Configured through menuconfig
 */
void ky036_init(void);

/**
 * @brief Check touch status
 * @return true when touched, false when not touched
 */
bool ky036_is_touched(void);

/**
 * @brief Read analog value
 */
int ky036_read_analog(void);

/**
 * @brief Deinitialize
 */
void ky036_deinit(void);

#ifdef __cplusplus
}
#endif