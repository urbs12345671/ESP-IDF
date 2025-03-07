#ifndef __TTP223_H__
#define __TTP223_H__

#include "driver/gpio.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        gpio_num_t gpio_num;
        bool touched;
    } touch_event_t;

    typedef struct
    {
        gpio_num_t gpio_num; // GPIO connected to TTP223's OUT pin
        bool pull_up_en;     // Enable internal pull-up
        bool pull_down_en;   // Enable internal pull-down
    } ttp223_config_t;

    esp_err_t ttp223_init(const ttp223_config_t *config);
    bool ttp223_read(gpio_num_t gpio_num);
    esp_err_t ttp223_register_isr_handler(gpio_num_t gpio_num);
    esp_err_t ttp223_get_event(touch_event_t *evt, TickType_t xTicksToWait);

#ifdef __cplusplus
}
#endif

#endif // __TTP223_H__