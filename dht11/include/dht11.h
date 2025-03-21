#pragma once

#include <stdbool.h>
#include <stdint.h>

/** Initialize DHT11
 * @param gpio_pin DHT11 GPIO pin
 * @return None
 */
void dht11_init(uint8_t gpio_pin);

/** Read data from DHT11
 * @param temperature Temperature value from DHT11
 * @param humidity Humidity value from DHT11
 * @return true if DHT11 data read is successful, and false otherwith
 */
bool dht11_read(float *temperature, float *humidity);