#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_camera.h"
#include "esp_http_client.h"
#include "wifi_controller.h"
#include "camera.h"

static const char *TAG = "main";

static const char *HTTP_POST_URL = "http://192.168.1.160:8080/upload.php";

static esp_err_t http_event_handler(esp_http_client_event_t *evt) {
    switch ((esp_http_client_event_id_t)(evt->event_id)) {
      case HTTP_EVENT_ERROR:
        ESP_LOGE(TAG, "HTTP_EVENT_ERROR");;
        break;
      case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
      case HTTP_EVENT_HEADER_SENT:
        ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
      case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
      case HTTP_EVENT_ON_DATA:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        if (!esp_http_client_is_chunked_response(evt->client)) {
          // Write out data
          // printf("%.*s", evt->data_len, (char*)evt->data);
        }
        break;
      case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
        break;
      case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
      default:
        break;  
    }

    return ESP_OK;
}

static int upload_picture(camera_fb_t *fb) {
    int fb_length = fb->len;

    esp_http_client_config_t config_client = {
        .url = HTTP_POST_URL,
        .event_handler = http_event_handler,
        .method = HTTP_METHOD_POST,
        .disable_auto_redirect = true,
        .timeout_ms = 180000,
        .is_async = false,
        .use_global_ca_store = false
    };
    esp_http_client_handle_t http_client = esp_http_client_init(&config_client);
    
    if (http_client) {
        // Set the POST request headers
        const static char content_type[] = "Content-Type";  
        const static char image_jpeg[] = "image/jpeg"; 
        esp_http_client_set_header(http_client, content_type, image_jpeg);
        char content_length[16];
        snprintf(content_length, sizeof(content_length), "%zu", fb_length);
        esp_http_client_set_header(http_client, "Content-Length", content_length);

        // Set the POST field
        esp_http_client_set_post_field(http_client, (const char *)fb->buf, fb->len);

        // POST
        esp_http_client_perform(http_client);
        ESP_LOGI(TAG, "esp_http_client_get_status_code: %d", esp_http_client_get_status_code(http_client));

        // Clean up
        esp_http_client_cleanup(http_client); 
    } else {
        ESP_LOGE(TAG, "Initialize http client failed");
    }

    return fb->len;
}

static void worker_task(void *pvParameters) {
    for (;;) {
        esp_log_level_set("ESP_HTTP_CLIENT", ESP_LOG_DEBUG);

        ESP_LOGI(TAG, "Taking picture...");
        camera_fb_t *picture = esp_camera_fb_get();
        if (picture && picture->buf) {
            ESP_LOGI(TAG, "Picture taken successfully. Size: %zu bytes", picture->len);
            
            int uploaded_length = upload_picture(picture);
            if (uploaded_length) {
                ESP_LOGI(TAG, "Upload successfully. Size: %zu bytes", uploaded_length);
            }
        } else {
            ESP_LOGI(TAG, "Taking picture failed");
        }
        esp_camera_fb_return(picture);

        // monitoring free heap
        ESP_LOGI(TAG, "Free heap size: %zu bytes", (uint)esp_get_free_heap_size());

        vTaskDelay(pdMS_TO_TICKS(60000));
    }
}

static void on_wifi_connected(esp_netif_ip_info_t *ip_info)
{
    if (!ip_info) {
        ESP_LOGE(TAG, "Wifi connected with NULL IP info");
        return;
    }
    ESP_LOGI(TAG, "Wifi connected! IP: " IPSTR, IP2STR(&ip_info->ip));

    // Initialize the camera
    if(ESP_OK != init_camera()) {
        return;
    } else {
        ESP_LOGI(TAG, "Camera initialized successfully");
    }

    // Create worker task with higher priority to handle picture taken and upload works
    xTaskCreate(worker_task, "worker_task", 4096, NULL, 4, NULL);
}

static void on_wifi_disconnected(void)
{
    ESP_LOGI(TAG, "Wifi disconnected");
}

static void on_wifi_connecting(uint8_t attempt) {
    ESP_LOGI(TAG, " Wifi Connecting attempt %d/%d", attempt, CONFIG_WIFI_MAX_RETRIES);
}

void app_main(void)
{
    // Initialize NVS if needed
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // Set up Wi-Fi callbacks
    wifi_event_callbacks_t wifi_callbacks = {
        .on_connected = on_wifi_connected,
        .on_disconnected = on_wifi_disconnected,
        .on_connecting = on_wifi_connecting
    };

    // Initialize Wifi
    wifi_init(&wifi_callbacks);     

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}