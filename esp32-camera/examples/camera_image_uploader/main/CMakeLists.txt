idf_component_register(
    SRCS "camera.c" "main.c"
    INCLUDE_DIRS "."
    PRIV_REQUIRES nvs_flash esp_psram esp_http_client)
