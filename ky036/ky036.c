#include "ky036.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali_scheme.h" 
#include "esp_adc/adc_cali.h"
#include "sdkconfig.h"

static const char *TAG = "KY036";
static gpio_num_t touch_gpio;
static adc_oneshot_unit_handle_t adc_handle = NULL;
static adc_channel_t analog_channel;
static adc_cali_handle_t cali_handle = NULL;

// ADC Calibration
static bool adc_calibration_init(void)
{
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,  
        .bitwidth = ADC_BITWIDTH_12,
    };
    return adc_cali_create_scheme_line_fitting(&cali_config, &cali_handle) == ESP_OK;
}

void ky036_init(void)
{
    // Digital init
    touch_gpio = CONFIG_KY036_TOUCH_GPIO;
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << touch_gpio),    
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE, 
        .pull_up_en = GPIO_PULLUP_DISABLE
    };
    gpio_config(&io_conf);

    // Analog init
    #ifdef CONFIG_KY036_USE_ANALOG
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));
    ESP_LOGI(TAG, "adc_oneshot_new_unit call succeed");

    adc_oneshot_chan_cfg_t adc_config = {
        .atten = ADC_ATTEN_DB_12, 
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, CONFIG_KY036_ANALOG_CHANNEL, &adc_config));
    ESP_LOGI(TAG, "adc_oneshot_config_channel call succeed");
    
    if(!adc_calibration_init()) {
        ESP_LOGE(TAG, "ADC calibration failed!");
    }
    #endif

    ESP_LOGI(TAG, "Initialized (Digital: GPIO%d, Analog: ADC1_CH%d)", 
             touch_gpio, CONFIG_KY036_ANALOG_CHANNEL);
}

bool ky036_is_touched(void)
{
    int raw = gpio_get_level(touch_gpio);
    ESP_LOGI(TAG, "ADC digital raw value %d", raw);

    return raw == 1;
}

int ky036_read_analog(void)
{
    #ifdef CONFIG_KY036_USE_ANALOG
    int raw = 0;
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, analog_channel, &raw));
    ESP_LOGI(TAG, "ADC analog raw value %d", raw);
    
    if(cali_handle) {
        int voltage = 0;
        adc_cali_raw_to_voltage(cali_handle, raw, &voltage);
        ESP_LOGI(TAG, "ADC analog raw value %d and voltage %d", raw, voltage);
        return voltage;  // Return millivolts
    }
    return raw;  // Fallback to raw value
    #else
    return -1;
    #endif
}

void ky036_deinit(void) {
    // Digital GPIO cleanup (optional)
    gpio_reset_pin(touch_gpio);
    
    // Analog ADC cleanup
    #ifdef CONFIG_KY036_USE_ANALOG
    if(adc_handle) {
        adc_oneshot_del_unit(adc_handle);
        adc_handle = NULL;
    }
    #endif
}