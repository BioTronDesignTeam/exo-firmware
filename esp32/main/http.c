#include "http.h"

#include <string.h>
#include "esp_http_client.h"
#include "esp_log.h"
#include "esp_tls.h"
#include "esp_crt_bundle.h"

static const char *TAG = "http_post";

// ─── Response buffer (shared across one request at a time) ───────────────────
static char     s_response_buf[512];
static int      s_response_len = 0;
static int      s_status_code  = -1;

// ─── ESP HTTP client event handler ───────────────────────────────────────────

static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {

    case HTTP_EVENT_ON_HEADER:
        // Headers available here if needed:
        // ESP_LOGD(TAG, "Header: %s: %s", evt->header_key, evt->header_value);
        break;

    case HTTP_EVENT_ON_DATA:
        {
            int copy_len = evt->data_len;
            int remaining = (int)sizeof(s_response_buf) - s_response_len - 1;
            if (copy_len > remaining) {
                copy_len = remaining;
            }
            if (copy_len > 0) {
                memcpy(s_response_buf + s_response_len, evt->data, copy_len);
                s_response_len += copy_len;
                s_response_buf[s_response_len] = '\0';
            }
        }
        break;

    case HTTP_EVENT_ON_FINISH:
        s_status_code = esp_http_client_get_status_code(evt->client);
        break;

    case HTTP_EVENT_ERROR:
        ESP_LOGW(TAG, "HTTP event error");
        break;

    default:
        break;
    }
    return ESP_OK;
}

// ─── Public implementation ────────────────────────────────────────────────────

http_result_t http_post_json(const char *json_payload)
{
    http_result_t result = {
        .status_code = -1,
        .success     = false,
        .body        = {0},
    };

    // Reset shared state
    memset(s_response_buf, 0, sizeof(s_response_buf));
    s_response_len = 0;
    s_status_code  = -1;

    esp_http_client_config_t config = {
    .host                        = HTTP_HOST,
    .path                        = HTTP_PATH,
    .port                        = HTTP_PORT,
    .transport_type              = HTTP_TRANSPORT_OVER_SSL,
    .event_handler               = http_event_handler,
    .timeout_ms                  = HTTP_TIMEOUT_MS,
    .crt_bundle_attach           = esp_crt_bundle_attach,
    .skip_cert_common_name_check = false,
};

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to init HTTP client");
        return result;
    }

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    // "Connection: close" avoids keep-alive stalls on eduroam
    esp_http_client_set_header(client, "Connection", "close");
    esp_http_client_set_post_field(client, json_payload, strlen(json_payload));

    ESP_LOGI(TAG, "POST → %s%s  payload: %s", HTTP_HOST, HTTP_PATH, json_payload);

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        result.status_code = esp_http_client_get_status_code(client);
        result.success     = (result.status_code >= 200 && result.status_code < 300);
        strncpy(result.body, s_response_buf, sizeof(result.body) - 1);
        ESP_LOGI(TAG, "Response %d: %s", result.status_code, result.body);
    } else {
        ESP_LOGE(TAG, "Request failed: %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
    return result;
}

void http_get_request(void) {
    esp_http_client_config_t config = {
        .url = "http://httpbin.org",
        .event_handler = http_event_handler,
    };
    
    // 1. Initialize client handle
    esp_http_client_handle_t client = esp_http_client_init(&config);

    // 2. Perform the request (Blocks until complete)
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP GET Status = %d, Length = %"PRId64,
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
    }

    // 3. Clean up resources
    esp_http_client_cleanup(client);
}