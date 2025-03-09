#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_log.h"
#include "ky026.h"

void app_main() {
    ky026_config_t config = {
        .digital_pin = GPIO_NUM_4,
        .digital_active_low = true,
        .adc_unit = ADC_UNIT_2,           // Safer choice with Wi-Fi
        .analog_channel = ADC_CHANNEL_6,  // GPIO34 (ADC1_CH6)
        .analog_atten = ADC_ATTEN_DB_12,  // Use DB_12 per v5.4 documentation 
        .bit_width = ADC_BITWIDTH_12,
        .analog_threshold = 2000,
    };   

    ky026_handle_t sensor = ky026_init(&config);
    if (!sensor) {
        ESP_LOGE("Main", "Initialization failed");
        return;
    }

    while (1) {
         // For ADC2 with Wi-Fi: Explicit locking
         ky026_adc2_lock(sensor);
         int analog_val = ky026_read_analog_raw(sensor);
         ky026_adc2_unlock(sensor);

        bool digital_state = ky026_read_digital(sensor);
        
        printf("Flame: %s | Analog: %d\n",
               digital_state ?  "CLEAR" : "DETECTED", // digital_active_low = true
               analog_val);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}