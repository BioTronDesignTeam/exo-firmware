#include "websockets.h"
#include "esp_log.h"
#include "esp_websocket_client.h"
#include "uart_consts.h"

static const char *TAG = "WEBSOCKET";

static esp_websocket_client_handle_t client = NULL;

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    static bool ws_connected;
    switch (event_id) {
        case WEBSOCKET_EVENT_CONNECTED:
            ws_connected = true;
            ESP_LOGI(TAG, "WebSocket connected");
            break;
        case WEBSOCKET_EVENT_DISCONNECTED:
            ws_connected = false;
            ESP_LOGI(TAG, "WebSocket disconnected");
            break;
        case WEBSOCKET_EVENT_DATA:
            if (ws_connected && data->data_ptr && data->data_len > 0) {
                // copy to a null-terminated buffer before printing
                char *buf = malloc(data->data_len + 1);
                if (buf) {
                    memcpy(buf, data->data_ptr, data->data_len);
                    buf[data->data_len] = '\0';
                    ESP_LOGI(TAG, "Received: %s", buf);
                    uart_write_bytes(UART_WEBSOCKET_PORT, data->data_ptr, data->data_len);
                    free(buf);
                }
            }
            break;
        case WEBSOCKET_EVENT_ERROR:
            ESP_LOGE(TAG, "WebSocket error");
            break;
        default:
            break;
    }
}

void websocket_init(const char* uri) {
    esp_websocket_client_config_t cfg = {
    .uri = uri,
    .skip_cert_common_name_check = true,
    .use_global_ca_store = false,
    .cert_pem = NULL,
    };

    client = esp_websocket_client_init(&cfg);
    esp_websocket_register_events(client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)client);
    esp_websocket_client_start(client);
}

void websocket_send(const char* message) {
    if (client) {
        esp_websocket_client_send_text(client, message, strlen(message), portMAX_DELAY);
    } else {
        ESP_LOGW(TAG, "WebSocket client not initialized");
    }
}

void websocket_stop() {
    if (client) {
        esp_websocket_client_stop(client);
        esp_websocket_client_destroy(client);
        client = NULL;
    }
}