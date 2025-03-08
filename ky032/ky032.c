#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_err.h"
#include "ky032.h"

static const char *TAG = "KY032";

esp_err_t ky032_init(gpio_num_t obstacle_pin, gpio_num_t en_pin)
{
    // Validate GPIOs
    if (!GPIO_IS_VALID_GPIO(obstacle_pin) || !GPIO_IS_VALID_OUTPUT_GPIO(en_pin)) {
        ESP_LOGE(TAG, "Invalid GPIOs: obstacle=%d, en=%d", obstacle_pin, en_pin);
        return ESP_ERR_INVALID_ARG;
    }

    // Configure obstacle detection pin
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << obstacle_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Obstacle pin config failed: %s", esp_err_to_name(ret));
        return ret;
    }

    // Configure enable pin
    gpio_reset_pin(en_pin);
    gpio_set_direction(en_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(en_pin, 1); // Enable by default

    ESP_LOGI(TAG, "Initialized sensor on OUT:%d, EN:%d", obstacle_pin, en_pin);

    return ESP_OK;
}

void ky032_enable(gpio_num_t en_pin) {
    gpio_set_direction(en_pin, GPIO_MODE_OUTPUT);
}

void ky032_disable(gpio_num_t en_pin) {
    gpio_set_direction(en_pin, GPIO_MODE_OUTPUT);
}

bool ky032_read(gpio_num_t obstacle_pin)
{
    return gpio_get_level(obstacle_pin);
}

esp_err_t ky032_install_isr(gpio_num_t obstacle_pin,
                           gpio_int_type_t intr_type,
                           void (*isr_handler)(void*),
                           void* args)
{
    esp_err_t ret = gpio_set_intr_type(obstacle_pin, intr_type);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set interrupt type: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = gpio_isr_handler_add(obstacle_pin, isr_handler, args);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add ISR handler: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

