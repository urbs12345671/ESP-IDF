#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"
#include "esp_check.h"
#include "ky026.h"

static const char *TAG = "KY026";

typedef struct {
    gpio_num_t digital_pin;
    bool digital_active_low;
    adc_unit_t adc_unit;
    adc_oneshot_unit_handle_t adc_handle;
    adc_channel_t analog_channel;
    adc_cali_handle_t cali_handle;
    int analog_threshold;
} ky026_dev_t;

ky026_handle_t ky026_init(const ky026_config_t *config) {
    ky026_dev_t *dev = NULL;

err: // Ensure clean up us done before return null
    if (dev) {
        if (dev->adc_handle) adc_oneshot_del_unit(dev->adc_handle);
        free(dev);
        return NULL;
    }   

    if (!config) {
        ESP_LOGE(TAG, "Invalid configuration");
        goto err;
    }
    
    if (config->adc_unit != ADC_UNIT_1 && config->adc_unit != ADC_UNIT_2) {
        ESP_LOGE(TAG, "Invalid ADC unit");
        goto err;
    }

    const adc_atten_t valid_atten[] = {
        ADC_ATTEN_DB_0, ADC_ATTEN_DB_2_5, 
        ADC_ATTEN_DB_6, ADC_ATTEN_DB_12
    };
    bool valid = false;
    for (int i = 0; i < sizeof(valid_atten)/sizeof(valid_atten[0]); i++) {
        if (config->analog_atten == valid_atten[i]) {
            valid = true;
            break;
        }
    }
    if (!valid) {
        ESP_LOGE(TAG, "Invalid attenuation");
        goto err;
    }

    // Add warning for DB_11 usage
    if (config->analog_atten == ADC_ATTEN_DB_11) {
        ESP_LOGW(TAG, "ADC_ATTEN_DB_11 is deprecated. Migrate to DB_12");
    }    

    // Allocate memory for device structure
    dev = calloc(1, sizeof(ky026_dev_t));
    if (!dev) {
        ESP_LOGE(TAG, "Memory allocation failed");
        goto err;
    }
    
    // Copy configuration parameters
    *dev = (ky026_dev_t){
        .digital_pin = config->digital_pin,
        .digital_active_low = config->digital_active_low,
        .adc_unit = config->adc_unit,
        .analog_channel = config->analog_channel,
        .analog_threshold = config->analog_threshold,
    };

    // Configure digital GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << dev->digital_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = dev->digital_active_low ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    if (gpio_config(&io_conf) != ESP_OK) {
        ESP_LOGE(TAG, "GPIO config failed");
        goto err;
    }

    // ADC Oneshot initialization
    adc_oneshot_unit_init_cfg_t adc_config = {
        .unit_id = dev->adc_unit,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    if (adc_oneshot_new_unit(&adc_config, &dev->adc_handle) != ESP_OK) {
        ESP_LOGE(TAG, "ADC init failed");
        goto err;
    }

    // Channel configuration
    adc_oneshot_chan_cfg_t chan_config = {
        .atten = config->analog_atten,
        .bitwidth = config->bit_width,
    };
    if (adc_oneshot_config_channel(dev->adc_handle, 
        dev->analog_channel, 
        &chan_config) != ESP_OK) {
        ESP_LOGE(TAG, "ADC channel config failed");
        goto err;
    }   

    // Initialize calibration (automatic cleanup in v5.4)
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = dev->adc_unit,
        .atten = config->analog_atten,
        .bitwidth = config->bit_width,
    };
    if (adc_cali_create_scheme_line_fitting(&cali_config, &dev->cali_handle) != ESP_OK) {
        ESP_LOGW(TAG, "No calibration scheme available for this ADC config");
    }    

    // Safety warnings
#if CONFIG_ESP_WIFI_ENABLED || CONFIG_BT_ENABLED
    if (dev->adc_unit == ADC_UNIT_1 && 
        (dev->analog_channel == ADC_CHANNEL_0 || dev->analog_channel == ADC_CHANNEL_3)) {
        ESP_LOGW(TAG, "ADC1 CH0/CH3 with Wi-Fi/BT may cause instability!");
    }
    if (dev->adc_unit == ADC_UNIT_2) {
        ESP_LOGW(TAG, "ADC2 requires locking when used with Wi-Fi");
    }
#endif

    return (ky026_handle_t)dev;
}

bool ky026_read_digital(ky026_handle_t handle) {
    ky026_dev_t *dev = (ky026_dev_t *)handle;
    int level = gpio_get_level(dev->digital_pin);
    return (dev->digital_active_low) ? (level == 0) : (level == 1);
}

int ky026_read_analog_raw(ky026_handle_t handle) {
    ky026_dev_t *dev = (ky026_dev_t *)handle;
    int raw_value;
    ESP_ERROR_CHECK(adc_oneshot_read(dev->adc_handle, dev->analog_channel, &raw_value));
    return raw_value;
}

bool ky026_analog_above_threshold(ky026_handle_t handle) {
    ky026_dev_t *dev = (ky026_dev_t *)handle;
    return ky026_read_analog_raw(handle) > dev->analog_threshold;
}

void ky026_set_analog_threshold(ky026_handle_t handle, int threshold) {
    ky026_dev_t *dev = (ky026_dev_t *)handle;
    dev->analog_threshold = threshold;
}

void ky026_adc2_lock(ky026_handle_t handle) {
#if SOC_ADC_DIG_CTRL_SUPPORTED && !SOC_ADC_RTC_CTRL_SUPPORTED
    ky026_dev_t *dev = (ky026_dev_t *)handle;
    if (dev->adc_unit == ADC_UNIT_2) {
        ESP_ERROR_CHECK(adc_oneshot_acquire(dev->adc_handle));
    }
#endif
}

void ky026_adc2_unlock(ky026_handle_t handle) {
#if SOC_ADC_DIG_CTRL_SUPPORTED && !SOC_ADC_RTC_CTRL_SUPPORTED
    ky026_dev_t *dev = (ky026_dev_t *)handle;
    if (dev->adc_unit == ADC_UNIT_2) {
        adc_oneshot_release(dev->adc_handle);
    }
#endif
}