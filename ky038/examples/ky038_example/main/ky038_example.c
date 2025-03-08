#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "ky038.h"

#define ADC_UNIT ADC_UNIT_1
#define ADC_CHANNEL ADC_CHANNEL_4
#define DIGITAL_PIN GPIO_NUM_4
#define ADC_ATTEN ADC_ATTEN_DB_12 

ky038_handle_t ky038;
volatile bool sound_triggered = false;

void IRAM_ATTR sound_detected(void)
{
    sound_triggered = true;
}

void sound_task(void *arg)
{
    while (1)
    {
        if (sound_triggered)
        {
            printf("Sound detected!\n");
            sound_triggered = false;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void)
{
    ky038_config_t config = {
        .adc_unit = ADC_UNIT,
        .adc_channel = ADC_CHANNEL,
        .adc_atten = ADC_ATTEN,
        .digital_pin = DIGITAL_PIN,
        .pull_up_en = true,
        .pull_down_en = false};

    // Init KY038
    ESP_ERROR_CHECK(ky038_init(&ky038, &config));

    gpio_install_isr_service(0);
    ESP_ERROR_CHECK(ky038_register_isr_handler(&ky038, GPIO_INTR_NEGEDGE, sound_detected));

    xTaskCreate(sound_task, "sound_task", 2048, NULL, 5, NULL);

    while (1)
    {
        printf("Digital: %d\n", ky038_read_digital(&ky038));
        printf("Analog: %d\n", ky038_read_analog(&ky038));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
