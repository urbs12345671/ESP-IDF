#include <stdio.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_netif.h"
#include "wifi_controller.h"

static const char *TAG = "MAIN";
static const char *BROKER_URI = "mqtt://mqtt.eclipseprojects.io";
static const char *CLIENT_ID = "esp32_client_dev1";
static const char *PUBLISH_TOPIC = "topic/pub";
//static const char *STATUS_TOPIC = "topic/status";
static const char *SUBSCRIBE_TOPIC = "topic/esp32_client_dev1";

static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqtt_connected = false;

/*------------ Begin MQTT event handler ------------*/
static void mqtt_event_handler(void *args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        mqtt_connected = true;
        ESP_LOGI(TAG, "MQTT Connected to broker");
        // Subscribe to MQTT topic
        esp_mqtt_client_subscribe(event->client, SUBSCRIBE_TOPIC, 1);
        // Publish a hello message
        esp_mqtt_client_publish(event->client, PUBLISH_TOPIC, "Hello", 5, 1, 0);
        break;

    case MQTT_EVENT_DISCONNECTED:
        mqtt_connected = false;
        ESP_LOGI(TAG, "MQTT Disconnected");

        //Clean up
        esp_mqtt_client_destroy(mqtt_client);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT Message [%.*s]: %.*s", event->topic_len, event->topic, event->data_len, event->data);
        
        /*-------------Begin handle MQTT messages---------------------------------------------*/  
        /* This is where specific task will be started to handle application specific logic based on the topic and message received */
    
        int matching_topic = -1; 
        // Create a null-terminated copy of the topic
        char *topic = malloc(event->topic_len + 1);
        if (topic) {
            memcpy(topic, event->topic, event->topic_len);
            topic[event->topic_len] = '\0'; // Null-terminate
            ESP_LOGI("MQTT", "Topic copy: %s", topic);
            matching_topic =  strcmp(topic, SUBSCRIBE_TOPIC);
            ESP_LOGI(TAG, "Matching topic = %d", matching_topic);
            free(topic); // Free allocated memory
        }
        if (matching_topic == 0)
        {
            ESP_LOGE(TAG, "GPIO action %.*s started...", event->data_len, event->data);
        }

        /*-------------Begin handle MQTT messages---------------------------------------------*/  
        break;

    case MQTT_EVENT_ERROR:
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_ESP_TLS)
        {
            ESP_LOGE(TAG, "MQTT Error: 0x%x (%s)",
                     event->error_handle->esp_tls_last_esp_err,
                     esp_err_to_name(event->error_handle->esp_tls_last_esp_err));
        }
        else
        {
            ESP_LOGE(TAG, "MQTT Protocol Error: 0x%x",
                     event->error_handle->connect_return_code);
        }
        break;  

    default:
        //ESP_LOGI(TAG, "Unhandled MQTT event: %d", event->event_id);
        break;
    }
}
/*------------ End MQTT event handler ------------*/

static void connect_mqtt(void)
{
   // MQTT Configuration
   esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
        .address.uri = BROKER_URI,
        },
        .credentials = {
            .client_id = CLIENT_ID,
        },
    };

    // Initialize MQTT client if not already initialized
    if (!mqtt_client)
    {
        mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
        esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
        esp_mqtt_client_start(mqtt_client);
    }
}

/*------------ Begin WiFi callbacks ------------*/
static void on_wifi_disconnected(void)
{
    ESP_LOGI(TAG, "Wi-Fi Disconnected! Stopping MQTT...");

    // Clean up MQTT client
    if (mqtt_client)
    {
        esp_mqtt_client_stop(mqtt_client);
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
    }
}

static void on_wifi_connected(esp_netif_ip_info_t *ip_info)
{
    ESP_LOGI(TAG, "Wi-Fi Connected! Starting MQTT...");

    connect_mqtt();
}

static void on_wifi_connecting(uint8_t attempt)
{
    ESP_LOGI(TAG, "Connection attempt %d/%d", attempt, CONFIG_WIFI_MAX_RETRIES);
}

/*------------ End WiFi callbacks ------------*/

void app_main()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Set up Wi-Fi callbacks
    wifi_event_callbacks_t wifi_callbacks = {
    .on_connected = on_wifi_connected,
    .on_disconnected = on_wifi_disconnected,
    .on_connecting = on_wifi_connecting};

    // Initialize Wi-Fi component
    wifi_init(&wifi_callbacks);

    // Main loop
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));

        // Periodic publish client status message
        if (mqtt_connected) {
            //int msg_id = esp_mqtt_client_publish(mqtt_client, STATUS_TOPIC, "online", 6, 1, 0);
            //if (msg_id == 0)
            //{
            //    ESP_LOGI(TAG, "Status message ID = %d", msg_id);
            //} 
        }
    }
}