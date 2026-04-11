idf_component_register(
    SRCS "main.c" "wifi.c" "websockets.c" "handle_uart.c"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi nvs_flash esp_event esp_websocket_client
)