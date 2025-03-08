#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_err.h"
#include "hal/adc_types.h"
#include "ky023.h"

static const char *TAG = "KY023";
static adc_oneshot_unit_handle_t adc_handle = NULL;
static adc_channel_t x_channel;
static adc_channel_t y_channel;
static gpio_num_t btn_gpio;

static esp_err_t gpio_to_adc1(gpio_num_t gpio, adc_channel_t *channel) {
    const struct { gpio_num_t gpio; adc_channel_t ch; } map[] = {
        {GPIO_NUM_32, ADC_CHANNEL_4},
        {GPIO_NUM_33, ADC_CHANNEL_5},
        {GPIO_NUM_34, ADC_CHANNEL_6},
        {GPIO_NUM_35, ADC_CHANNEL_7},
        {GPIO_NUM_36, ADC_CHANNEL_0},
        {GPIO_NUM_37, ADC_CHANNEL_1},
        {GPIO_NUM_38, ADC_CHANNEL_2},
        {GPIO_NUM_39, ADC_CHANNEL_3},
    };

    for (size_t i = 0; i < sizeof(map)/sizeof(map[0]); i++) {
        if (gpio == map[i].gpio) {
            *channel = map[i].ch;
            return ESP_OK;
        }
    }
    return ESP_ERR_INVALID_ARG;
}

esp_err_t ky023_init(const ky023_config_t *config) {
    // Convert GPIOs to ADC channels
    ESP_RETURN_ON_ERROR(gpio_to_adc1(config->x_gpio, &x_channel), 
                      TAG, "Invalid X GPIO");
    ESP_RETURN_ON_ERROR(gpio_to_adc1(config->y_gpio, &y_channel), 
                      TAG, "Invalid Y GPIO");

    // ADC initialization
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
        .clk_src = ADC_DIGI_CLK_SRC_DEFAULT,  // Required for v5.x
    };
    
    ESP_RETURN_ON_ERROR(adc_oneshot_new_unit(&init_config, &adc_handle), 
                      TAG, "ADC unit init failed");

    // Channel configuration
    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = config->attenuation,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    
    ESP_RETURN_ON_ERROR(adc_oneshot_config_channel(adc_handle, x_channel, &chan_cfg),
                      TAG, "X channel config failed");
    ESP_RETURN_ON_ERROR(adc_oneshot_config_channel(adc_handle, y_channel, &chan_cfg),
                      TAG, "Y channel config failed");

    // Button GPIO configuration
    btn_gpio = config->button_gpio;
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << btn_gpio),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_RETURN_ON_ERROR(gpio_config(&io_conf), 
                      TAG, "Button GPIO config failed");

    return ESP_OK;
}

uint16_t ky023_read_x(void) {
    int raw = 0;
    adc_oneshot_read(adc_handle, x_channel, &raw);
    return (uint16_t)raw;
}

uint16_t ky023_read_y(void) {
    int raw = 0;
    adc_oneshot_read(adc_handle, y_channel, &raw);
    return (uint16_t)raw;
}

bool ky023_read_button(void) {
    return (gpio_get_level(btn_gpio) == 0);
}

void ky023_cleanup(void) {
    if (adc_handle) {
        adc_oneshot_del_unit(adc_handle);
        adc_handle = NULL;
    }
}