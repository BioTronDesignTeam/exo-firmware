#include "eduroam_wifi.h"
#include "http.h"
#include "handle_uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "main";

#define POST_INTERVAL_MS  30000

// placeholder, make read from uart function to replace this
static void build_payload(char *buf, size_t buf_len)
{
    float temperature = 22.4f;   // TODO: read from sensor
    int   humidity    = 55;      // TODO: read from sensor
    uint32_t uptime   = (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);

    snprintf(buf, buf_len,
             "{\"temperature\":%.1f,\"humidity\":%d,\"uptime_ms\":%lu}",
             temperature, humidity, (unsigned long)uptime);
}

// main task 
void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 eduroam HTTP POST starting");

    /*
    if (!wifi_connect()) {
        ESP_LOGE(TAG, "cannot connect to eduroam");
        return;
    }
    */

    ESP_LOGW(TAG, "UART intiialized");
    uart_init();
    ESP_LOGW(TAG, "Starting UART mirror task");
    xTaskCreate(&uart2_mirror_task, "uart2_mirror", 2048, NULL, 5, NULL);

    char payload[256];

    while (1) {
        /*
        if (!wifi_ensure_connected()) {
            ESP_LOGW(TAG, "wifi unavailable, retrying...");
            vTaskDelay(pdMS_TO_TICKS(POST_INTERVAL_MS));
            continue;
        }

        build_payload(payload, sizeof(payload));
        http_result_t result = http_post_json(payload);

        if (result.success) {
            ESP_LOGI(TAG, "POST OK (%d)", result.status_code);
        } else {
            ESP_LOGW(TAG, "POST failed (%d)", result.status_code);
        }
        
        */
       vTaskDelay(pdMS_TO_TICKS(POST_INTERVAL_MS));
    }
}