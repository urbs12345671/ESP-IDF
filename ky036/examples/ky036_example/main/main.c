#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ky036.h"

static const char *TAG = "main";

void app_main(void)
{
    ky036_init();
    
    while(1) {
        if(ky036_is_touched()) {
            printf("Touch detected!\n");
        } else {
            printf("No touch\n");
        }

        int analogValue = ky036_read_analog();
        ESP_LOGI(TAG, "Analog = %d", analogValue);

        vTaskDelay(pdMS_TO_TICKS(200));
    }

    ky036_deinit();
}