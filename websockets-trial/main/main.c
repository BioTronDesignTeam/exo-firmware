#include "wifi.h"
#include "nvs_flash.h"
#include "websockets.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "handle_uart.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/netdb.h"

void app_main(void)
{
    ESP_ERROR_CHECK(wifi_init());
    struct addrinfo *res;
    int err = getaddrinfo("ws://172.20.10.8:8765", NULL, NULL, &res);
    ESP_LOGI("DNS", "getaddrinfo result: %d", err);
    if (err == 0) freeaddrinfo(res);

    vTaskDelay(pdMS_TO_TICKS(1000));
    websocket_init("ws://172.20.10.8:8765");
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    websocket_send("testing from esp32 messaging");

    xTaskCreate(uart_sim_task, "uart_sim_task", 4096, NULL, 5, NULL);
}