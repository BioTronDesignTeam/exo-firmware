#include "eduroam_wifi.h"

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"

static const char *TAG = "wifi";

// Event group bits
#define WIFI_CONNECTED_BIT  BIT0
#define WIFI_FAIL_BIT       BIT1

static EventGroupHandle_t s_wifi_event_group = NULL;
static esp_netif_t       *s_sta_netif        = NULL;
static bool               s_initialised      = false;

// Event handler 

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();

    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t *d = (wifi_event_sta_disconnected_t *)event_data;
        ESP_LOGW(TAG, "Disconnected (reason %d)", d->reason);
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);

    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *e = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&e->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// init (run once) 

static esp_err_t wifi_init(void)
{
    // NVS is required by the WiFi driver
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    s_sta_netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                               &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                               &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    s_initialised = true;
    return ESP_OK;
}

// WPA2-Enterprise config 

static void wifi_configure_enterprise(void)
{
    //if (EAP_CA_CERT_PEM != NULL) {
    #ifdef EAP_CA_CERT_PEM
        ESP_ERROR_CHECK(esp_wifi_sta_wpa2_ent_set_ca_cert(
            (const unsigned char *)EAP_CA_CERT_PEM,
            strlen(EAP_CA_CERT_PEM) + 1));
    //}
    #endif

    ESP_ERROR_CHECK(esp_wifi_sta_wpa2_ent_set_identity(
        (const unsigned char *)EAP_IDENTITY, strlen(EAP_IDENTITY)));
    ESP_ERROR_CHECK(esp_wifi_sta_wpa2_ent_set_username(
        (const unsigned char *)EAP_USERNAME, strlen(EAP_USERNAME)));
    ESP_ERROR_CHECK(esp_wifi_sta_wpa2_ent_set_password(
        (const unsigned char *)EAP_PASSWORD, strlen(EAP_PASSWORD)));

    ESP_ERROR_CHECK(esp_wifi_sta_wpa2_ent_enable());
}

// Single connection attempt 

static bool wifi_try_connect(void)
{
    s_wifi_event_group = xEventGroupCreate();

    wifi_configure_enterprise();

    wifi_config_t wifi_cfg = {
        .sta = {
            .ssid = EAP_SSID,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_start());   // triggers WIFI_EVENT_STA_START → connect

    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE,    // don't clear on exit
        pdFALSE,    // wait for either bit
        pdMS_TO_TICKS(WIFI_CONNECT_TIMEOUT_MS));

    bool connected = (bits & WIFI_CONNECTED_BIT) != 0;

    if (!connected) {
        // Clean up for a retry
        esp_wifi_stop();
        esp_wifi_sta_wpa2_ent_disable();
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = NULL;
    }

    return connected;
}

// Public implementation 

bool wifi_connect(void)
{
    if (!s_initialised) {
        wifi_init();
    }

    // temporary scan snippet — delete after confirming eduroam is visible
esp_wifi_set_mode(WIFI_MODE_STA);
esp_wifi_start();

wifi_scan_config_t scan_cfg = {
    .ssid        = NULL,   // NULL = scan all
    .bssid       = NULL,
    .channel     = 0,
    .show_hidden = false,
    .scan_type   = WIFI_SCAN_TYPE_ACTIVE,
};
esp_wifi_scan_start(&scan_cfg, true);  // true = block until done

uint16_t ap_count = 0;
esp_wifi_scan_get_ap_num(&ap_count);
ESP_LOGI("scan", "Found %d APs", ap_count);

static wifi_ap_record_t ap_list[20];
uint16_t max = 20;
esp_wifi_scan_get_ap_records(&max, ap_list);

for (int i = 0; i < max; i++) {
    ESP_LOGI("scan", "[%d] ssid:%-32s  ch:%d  rssi:%d  auth:%d",
             i, ap_list[i].ssid, ap_list[i].primary,
             ap_list[i].rssi, ap_list[i].authmode);
}


    int attempts = 0;

    while (WIFI_MAX_RETRIES == 0 || attempts < WIFI_MAX_RETRIES) {
        attempts++;
        ESP_LOGI(TAG, "Connecting to '%s' (attempt %d/%d)...",
                 EAP_SSID, attempts, WIFI_MAX_RETRIES);

        if (wifi_try_connect()) {
            ESP_LOGI(TAG, "Connected successfully.");
            return true;
        }

        ESP_LOGW(TAG, "Attempt %d failed. Retrying in %d ms...",
                 attempts, WIFI_RETRY_DELAY_MS);
        vTaskDelay(pdMS_TO_TICKS(WIFI_RETRY_DELAY_MS));
    }

    ESP_LOGE(TAG, "Max retries reached. Could not connect.");
    return false;
}

bool wifi_ensure_connected(void)
{
    if (esp_netif_is_netif_up(s_sta_netif)) {
        return true;
    }
    ESP_LOGW(TAG, "Connection lost — reconnecting...");
    return wifi_connect();
}

void wifi_disconnect(void)
{
    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_sta_wpa2_ent_disable();
    esp_wifi_deinit();
    if (s_wifi_event_group) {
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = NULL;
    }
    ESP_LOGI(TAG, "Disconnected.");
}