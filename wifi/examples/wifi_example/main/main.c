#include "wifi_controller.h"
#include "nvs_flash.h"
#include "esp_log.h"

static const char *TAG = "MAIN";

static void on_wifi_connected(esp_netif_ip_info_t *ip_info)
{
    if (!ip_info) {
        ESP_LOGE(TAG, "Connected with NULL IP info");
        return;
    }
    ESP_LOGI(TAG, "Connected! IP: " IPSTR, IP2STR(&ip_info->ip));
    // Start MQTT client here
}

static void on_wifi_disconnected(void)
{
    ESP_LOGI(TAG, "Wi-Fi Disconnected");
}

static void on_wifi_connecting(uint8_t attempt) {
    ESP_LOGI(TAG, "Connection attempt %d/%d", 
           attempt, CONFIG_WIFI_MAX_RETRIES);
}

void app_main()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Set up Wi-Fi callbacks
    wifi_event_callbacks_t wifi_callbacks = {
        .on_connected = on_wifi_connected,
        .on_disconnected = on_wifi_disconnected,
        .on_connecting = on_wifi_connecting
    };

    // Initialize Wi-Fi
    wifi_init(&wifi_callbacks);

    // Application loop
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        if (wifi_is_connected()) {
            // Perform periodic tasks when connected
        }
    }
}