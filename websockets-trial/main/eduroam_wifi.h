#pragma once

#include <stdbool.h>

// Eduroam credentials
#define EAP_SSID        "eduroam"
#define EAP_IDENTITY    "waterloo-email"   // anonymous identity (outer)
#define EAP_USERNAME    "waterloo-email"   // inner identity
#define EAP_PASSWORD    "password"

// Set to NULL if your institution doesn't require a server certificate.
// If needed, paste the PEM string in eduroam_wifi.c and point this here.
//#define EAP_CA_CERT_PEM  NULL

// Timeouts / retry
#define WIFI_CONNECT_TIMEOUT_MS   15000
#define WIFI_RETRY_DELAY_MS        1000
#define WIFI_MAX_RETRIES              15    // 0 = retry forever

/**
 * Initialise the TCP/IP stack, create the default STA netif, and connect to
 * eduroam using WPA2-Enterprise (PEAP/MSCHAPv2).
 *
 * Blocks until connected or max retries exceeded.
 * Returns true on success, false on failure.
 */
bool wifi_connect(void);

/**
 * Check the link; reconnect if it has dropped.
 * Call this before every HTTP request.
 * Returns true if (re)connected, false if unreachable.
 */
bool wifi_ensure_connected(void);

/**
 * Tear down the WiFi connection and free resources.
 */
void wifi_disconnect(void);