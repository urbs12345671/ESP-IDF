#include <string.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "ky038.h"

static const char *TAG = "KY038";

static void IRAM_ATTR ky038_isr_handler(void *arg)
{
    ky038_handle_t *handle = (ky038_handle_t *)arg;
    if (handle->user_callback != NULL)
    {
        handle->user_callback();
    }
}

esp_err_t ky038_init(ky038_handle_t *handle, const ky038_config_t *config)
{
    esp_err_t ret = ESP_OK;

    // Validate configuration
    if (config->adc_unit != ADC_UNIT_1 && config->adc_unit != ADC_UNIT_2)
    {
        ESP_LOGE(TAG, "Invalid ADC unit");
        return ESP_ERR_INVALID_ARG;
    }

    if (!GPIO_IS_VALID_GPIO(config->digital_pin))
    {
        ESP_LOGE(TAG, "Invalid GPIO pin");
        return ESP_ERR_INVALID_ARG;
    }

    // Initialize ADC
    adc_oneshot_unit_init_cfg_t adc_config = {
        .unit_id = config->adc_unit,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ret = adc_oneshot_new_unit(&adc_config, &handle->adc_handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize ADC unit: %s", esp_err_to_name(ret));
        return ret;
    }

    // Configure ADC channel
    adc_oneshot_chan_cfg_t chan_config = {
        .atten = config->adc_atten,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret = adc_oneshot_config_channel(handle->adc_handle, config->adc_channel, &chan_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to configure ADC channel: %s", esp_err_to_name(ret));
        return ret;
    }

    // Configure digital input
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << config->digital_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = config->pull_up_en,
        .pull_down_en = config->pull_down_en,
        .intr_type = GPIO_INTR_DISABLE};
    ret = gpio_config(&io_conf);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to configure GPIO: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize handle state
    handle->config = *config;
    handle->user_callback = NULL;

    ESP_LOGI(TAG, "Initialized (ADC Unit: %d, Channel: %d, Digital Pin: %d)",
             config->adc_unit, config->adc_channel, config->digital_pin);
    return ESP_OK;
}

int ky038_read_analog(ky038_handle_t *handle)
{
    int raw_value;
    adc_oneshot_read(handle->adc_handle, handle->config.adc_channel, &raw_value);
    return raw_value; // Return raw ADC value
}

int ky038_read_digital(ky038_handle_t *handle)
{
    return gpio_get_level(handle->config.digital_pin);
}

esp_err_t ky038_register_isr_handler(ky038_handle_t *handle, gpio_int_type_t intr_type, ky038_isr_callback_t callback)
{
    esp_err_t ret;

    if (handle->user_callback != NULL)
    {
        ESP_LOGE(TAG, "ISR handler already registered");
        return ESP_ERR_INVALID_STATE;
    }

    ret = gpio_set_intr_type(handle->config.digital_pin, intr_type);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to set interrupt type: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = gpio_isr_handler_add(handle->config.digital_pin, ky038_isr_handler, handle);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add ISR handler: %s", esp_err_to_name(ret));
        return ret;
    }

    handle->user_callback = callback;
    ESP_LOGI(TAG, "Registered ISR handler for pin %d", handle->config.digital_pin);

    return ESP_OK;
}

esp_err_t ky038_deinit(ky038_handle_t *handle)
{
    if (handle->user_callback != NULL)
    {
        gpio_isr_handler_remove(handle->config.digital_pin);
    }

    adc_oneshot_del_unit(handle->adc_handle);
    gpio_reset_pin(handle->config.digital_pin);
    memset(handle, 0, sizeof(ky038_handle_t));
    ESP_LOGI(TAG, "Deinitialized");
    
    return ESP_OK;
}