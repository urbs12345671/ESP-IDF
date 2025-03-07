#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_attr.h"
#include "ttp223.h"

static const char *TAG = "TTP223";

static QueueHandle_t ttp223_event_queue = NULL;

typedef struct
{
    gpio_num_t gpio_num;
} ttp223_isr_arg_t;

static void IRAM_ATTR ttp223_isr_handler(void *arg)
{
    ttp223_isr_arg_t *isr_arg = (ttp223_isr_arg_t *)arg;
    bool level = gpio_get_level(isr_arg->gpio_num);

    touch_event_t evt = {
        .gpio_num = isr_arg->gpio_num,
        .touched = level};

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(ttp223_event_queue, &evt, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken)
    {
        portYIELD_FROM_ISR();
    }
}

esp_err_t ttp223_init(const ttp223_config_t *config)
{
    if (config == NULL)
    {
        ESP_LOGE(TAG, "Invalid configuration");
        return ESP_ERR_INVALID_ARG;
    }

    // Create event queue if not exists
    if (!ttp223_event_queue)
    {
        ttp223_event_queue = xQueueCreate(10, sizeof(touch_event_t));
        if (!ttp223_event_queue)
        {
            ESP_LOGE(TAG, "Failed to create event queue");
            return ESP_FAIL;
        }
    }

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << config->gpio_num),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = config->pull_up_en,
        .pull_down_en = config->pull_down_en,
        .intr_type = GPIO_INTR_DISABLE};
    return gpio_config(&io_conf);
}

bool ttp223_read(gpio_num_t gpio_num)
{
    return gpio_get_level(gpio_num);
}

esp_err_t ttp223_register_isr_handler(gpio_num_t gpio_num)
{
    ttp223_isr_arg_t *isr_arg = malloc(sizeof(ttp223_isr_arg_t));
    if (!isr_arg)
    {
        ESP_LOGE(TAG, "Memory allocation failed");
        return ESP_ERR_NO_MEM;
    }

    isr_arg->gpio_num = gpio_num;

    esp_err_t err = gpio_set_intr_type(gpio_num, GPIO_INTR_ANYEDGE);
    if (err != ESP_OK)
    {
        free(isr_arg);
        return err;
    }

    err = gpio_isr_handler_add(gpio_num, ttp223_isr_handler, isr_arg);
    if (err != ESP_OK)
    {
        free(isr_arg);
    }
    return err;
}

esp_err_t ttp223_get_event(touch_event_t *evt, TickType_t xTicksToWait)
{
    if (!ttp223_event_queue)
        return ESP_ERR_INVALID_STATE;
    return xQueueReceive(ttp223_event_queue, evt, xTicksToWait) ? ESP_OK : ESP_FAIL;
}