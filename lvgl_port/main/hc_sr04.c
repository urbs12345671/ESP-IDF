#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_private/esp_clk.h"
#include "driver/mcpwm_cap.h"
#include "driver/gpio.h"
#include "hc_sr04.h"

#define TAG "hc-sr04"

static void generate_trigger_pulse(gpio_num_t trig_gpio);
static bool echo_callback(mcpwm_cap_channel_handle_t cap_channel,
                          const mcpwm_capture_event_data_t *edata,
                          void *user_data);

bool hc_sr04_init(hc_sr04_t *config)
{
    // Validate input
    if (!config || !config->trig_gpio || !config->echo_gpio)
    {
        ESP_LOGE(TAG, "Invalid configuration");
        return false;
    }

    // Initialize capture timer
    mcpwm_capture_timer_config_t cap_timer_conf = {
        .clk_src = MCPWM_CAPTURE_CLK_SRC_DEFAULT,
        .group_id = 0,
    };

    if (mcpwm_new_capture_timer(&cap_timer_conf, &config->cap_timer) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to create capture timer");
        return false;
    }

    // Initialize capture channel
    mcpwm_capture_channel_config_t cap_chan_conf = {
        .gpio_num = config->echo_gpio,
        .prescale = 1,
        .flags = {
            .neg_edge = true,
            .pos_edge = true,
            .pull_up = true,
        },
    };
    if (mcpwm_new_capture_channel(config->cap_timer, &cap_chan_conf, &config->cap_channel) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to create capture channel");
        return false;
    }

    // Register callback
    mcpwm_capture_event_callbacks_t cbs = {
        .on_cap = echo_callback,
    };
    if (mcpwm_capture_channel_register_event_callbacks(config->cap_channel, &cbs, config) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to register callback");
        return false;
    }

    // Enable capture channel and timer
    if (mcpwm_capture_channel_enable(config->cap_channel) != ESP_OK ||
        mcpwm_capture_timer_enable(config->cap_timer) != ESP_OK ||
        mcpwm_capture_timer_start(config->cap_timer) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to enable capture");
        return false;
    }

    // Configure trigger GPIO
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << config->trig_gpio),
    };
    if (gpio_config(&io_conf) != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to configure trigger GPIO");
        return false;
    }

    config->task_handle = xTaskGetCurrentTaskHandle();

    return true;
}

bool hc_sr04_measure_distance(hc_sr04_t *config, float *distance_out, uint32_t timeout_ms)
{
    if (!config || !distance_out)
        return false;

    generate_trigger_pulse(config->trig_gpio);

    ESP_LOGI(TAG, "here1");

    uint32_t tof_ticks;
    if (xTaskNotifyWait(0, ULONG_MAX, &tof_ticks, pdMS_TO_TICKS(timeout_ms)))
    {
        ESP_LOGI(TAG, "here2");

        const float pulse_width_us = tof_ticks * (1000000.0 / esp_clk_apb_freq());

        if (pulse_width_us > 35000)
        {
            ESP_LOGI(TAG, "here3");
            return false; // Invalid reading
        }


        *distance_out = pulse_width_us / 58.0f;
        return true;
    }

    return false;
}

static void generate_trigger_pulse(gpio_num_t trig_gpio)
{
    gpio_set_level(trig_gpio, 1);
    esp_rom_delay_us(10);
    gpio_set_level(trig_gpio, 0);
}

static bool echo_callback(mcpwm_cap_channel_handle_t cap_chan,
                          const mcpwm_capture_event_data_t *edata,
                          void *user_data)
{
    static uint32_t cap_val_begin = 0;
    hc_sr04_t *config = (hc_sr04_t *)user_data;
    BaseType_t high_task_wakeup = pdFALSE;

    if (edata->cap_edge == MCPWM_CAP_EDGE_POS)
    {
        cap_val_begin = edata->cap_value;
    }
    else
    {
        const uint32_t pulse_ticks = edata->cap_value - cap_val_begin;
        xTaskNotifyFromISR(config->task_handle, pulse_ticks,
                           eSetValueWithOverwrite, &high_task_wakeup);
    }

    return high_task_wakeup == pdTRUE;
}