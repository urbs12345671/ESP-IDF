#ifndef KY032_H
#define KY032_H

#include "driver/gpio.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize KY-032 sensor with both OUT and EN pins
 * 
 * @param obstacle_pin GPIO for sensor OUT pin
 * @param en_pin GPIO for sensor EN pin (enable control)
 * @return esp_err_t 
 */
esp_err_t ky032_init(gpio_num_t obstacle_pin, gpio_num_t en_pin);

/**
 * @brief Enable the sensor (set EN pin HIGH)
 * 
 * @param en_pin GPIO used for EN pin
 */
void ky032_enable(gpio_num_t en_pin);

/**
 * @brief Disable the sensor (set EN pin LOW)
 * 
 * @param en_pin GPIO used for EN pin
 */
void ky032_disable(gpio_num_t en_pin);


/**
 * @brief Read obstacle detection status
 * 
 * @param obstacle_pin GPIO pin used for sensor
 * @return true - Obstacle detected
 * @return false - No obstacle
 */
bool ky032_read(gpio_num_t obstacle_pin);

/**
 * @brief Install ISR handler for sensor
 * 
 * @param obstacle_pin GPIO pin used for sensor
 * @param intr_type Interrupt type (e.g., GPIO_INTR_NEGEDGE)
 * @param isr_handler User-defined ISR handler function
 * @param args Arguments to pass to ISR handler
 * @return esp_err_t 
 */
esp_err_t ky032_install_isr(gpio_num_t obstacle_pin,
                           gpio_int_type_t intr_type,
                           void (*isr_handler)(void*),
                           void* args);                       

#ifdef __cplusplus
}
#endif

#endif // KY032_H