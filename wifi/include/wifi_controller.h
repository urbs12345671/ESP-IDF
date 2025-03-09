#pragma once
#include <stdbool.h>
#include "esp_mac.h"
#include "esp_event.h"
#include "esp_netif_ip_addr.h"
#include "esp_netif_types.h"

 #ifdef __cplusplus
 extern "C" {
 #endif
 
 /**
  * @brief Wi-Fi event callback structure
  * 
  * Contains function pointers for handling Wi-Fi connection events.
  * All callbacks are optional (can be set to NULL if not needed).
  */
 typedef struct {
     /**
      * @brief Called when successfully connected to AP and got IP address
      * @param ip_info Pointer to structure containing assigned IP information
      */
     void (*on_connected)(esp_netif_ip_info_t *ip_info);
 
     /**
      * @brief Called when disconnected from AP
      */
     void (*on_disconnected)(void);
 
     /**
      * @brief Called during connection attempts
      * @param attempt Current attempt number (1-based index)
      */
     void (*on_connecting)(uint8_t attempt);
 } wifi_event_callbacks_t;
 
 /**
  * @brief Initialize Wi-Fi connection manager
  * 
  * @param callbacks Structure containing event callbacks
  * @note Must be called once before any other wifi_* functions
  * @note Wi-Fi credentials should be configured via Kconfig (menuconfig)
  */
 void wifi_init(wifi_event_callbacks_t *callbacks);
 
 /**
  * @brief Manually trigger connection attempt
  * 
  * @note Normally not needed - automatic connection is handled by component
  */
 void wifi_connect(void);
 
 /**
  * @brief Disconnect from current AP
  */
 void wifi_disconnect(void);
 
 /**
  * @brief Check connection status
  * 
  * @return true if connected to AP and has valid IP
  * @return false otherwise
  */
 bool wifi_is_connected(void);
 
 /**
  * @brief Get current IP configuration
  * 
  * @return esp_netif_ip_info_t IP address information
  * @note Returns zeros if not connected
  */
 esp_netif_ip_info_t wifi_get_ip_info(void);
 
 #ifdef __cplusplus
 }
 #endif