#include "esp_websocket_client.h"
#include "esp_log.h"
#include "eduroam_wifi.h"

static const char* TAG = "WEBSOCKET_TASK";

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
        break;
    case WEBSOCKET_EVENT_DATA:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
        ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
        ESP_LOGW(TAG, "Received=%.*s", data->data_len, (char *)data->data_ptr);
        ESP_LOGW(TAG, "Total payload length=%d, data_len=%d, current payload offset=%d\r\n", data->payload_len, data->data_len, data->payload_offset);

        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        break;
    }
}

void websocket_task() {
    esp_websocket_client_config_t websocket_cfg = { //look into setting up our own websocket server
        .uri = "ws://ws.ifelse.io",
        .buffer_size = 1024,      // default is 512
        .headers = "Pragma: no-cache\r\nCache-Control: no-cache\r\n",
    };
    ESP_LOGI(TAG, "Starting WebSocket client");
    esp_websocket_client_handle_t client = esp_websocket_client_init(&websocket_cfg);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to init websocket client");
        return;
    }
    ESP_LOGI(TAG, "WebSocket client initialized");
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void*)client);
    ESP_LOGI(TAG, "WebSocket events registered");
    esp_websocket_client_start(client);
    ESP_LOGI(TAG, "started WebSocket client");

    char data[32];
    uint32_t i = 0;
    for ( ;; ) {
        ESP_LOGI(TAG, "debug check 1");
        if (!wifi_ensure_connected()) {
            ESP_LOGW(TAG, "wifi unavailable, retrying...");
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        if (esp_websocket_client_is_connected(client)) {
            ESP_LOGI(TAG, "debug check 2");
            uint32_t len = snprintf(data, sizeof(data), "Hello %ld", i++);
            esp_websocket_client_send_text(client, data, len, 0);
        }
        ESP_LOGI(TAG, "debug check 3");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    esp_websocket_client_stop(client);
    ESP_LOGI(TAG, "Websocket Stopped");
    esp_websocket_client_destroy(client);
}