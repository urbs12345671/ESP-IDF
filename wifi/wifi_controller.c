#include "wifi_controller.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

static const char *TAG = "WiFi";
static wifi_event_callbacks_t *user_callbacks = NULL;
static uint8_t connection_retries = 0;
static esp_netif_t *sta_netif = NULL;

static void event_handler(void *arg, esp_event_base_t event_base,
                         int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT) {
        switch (event_id) {
            case WIFI_EVENT_STA_START:
                ESP_LOGI(TAG, "STA Start");
                esp_wifi_connect();
                break;

            case WIFI_EVENT_STA_CONNECTED:
                ESP_LOGI(TAG, "Connected to AP");
                connection_retries = 0;
                break;

            case WIFI_EVENT_STA_DISCONNECTED:
                if (connection_retries < CONFIG_WIFI_MAX_RETRIES) {
                    ESP_LOGI(TAG, "Retrying in %dms (attempt %d/%d)", 
                            CONFIG_WIFI_RETRY_TIMEOUT_MS,
                            connection_retries + 1,
                            CONFIG_WIFI_MAX_RETRIES);
                    vTaskDelay(pdMS_TO_TICKS(CONFIG_WIFI_RETRY_TIMEOUT_MS));
                    esp_wifi_connect();
                    connection_retries++;
                    if (user_callbacks && user_callbacks->on_connecting) {
                        user_callbacks->on_connecting(connection_retries);
                    }
                } else {
                    ESP_LOGW(TAG, "Max retries (%d) reached", CONFIG_WIFI_MAX_RETRIES);
                    if (user_callbacks && user_callbacks->on_disconnected) {
                        user_callbacks->on_disconnected();
                    }
                }
                break;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        if (user_callbacks && user_callbacks->on_connected) {
            user_callbacks->on_connected(&event->ip_info);
        }
    }
}

void wifi_init(wifi_event_callbacks_t *callbacks)
{
    user_callbacks = callbacks;

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_WIFI_SSID,
            .password = CONFIG_WIFI_PASSWORD,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
}

bool wifi_is_connected(void)
{
    return esp_netif_is_netif_up(sta_netif);
}

esp_netif_ip_info_t wifi_get_ip_info(void)
{
    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(sta_netif, &ip_info);
    return ip_info;
}

void wifi_disconnect(void)
{
    esp_wifi_disconnect();
}

void wifi_connect(void)
{
    connection_retries = 0;
    esp_wifi_connect();
}