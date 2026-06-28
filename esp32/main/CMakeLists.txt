idf_component_register(
    SRCS "crc.c" "main.c" "http.c" "eduroam_wifi.c" "handle_uart.c"
    INCLUDE_DIRS "."
    REQUIRES esp_wifi nvs_flash esp_event esp_netif esp_http_client esp-tls wpa_supplicant esp_driver_uart
)