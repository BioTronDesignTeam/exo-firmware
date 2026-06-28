#pragma once

#include <stdbool.h>

// ─── Target endpoint ──────────────────────────────────────────────────────────
// HTTPS on port 443 is the safest choice — reliably open outbound on eduroam.
#define HTTP_HOST      "webhook.site"
#define HTTP_PATH      "/85aa14e7-f16b-4b9f-a646-7d0dbd2c3d6c"
#define HTTP_PORT      443
#define HTTP_USE_TLS   1     // set to 0 for plain HTTP on port 80

// Optional: your server's Root CA PEM for TLS verification.
// Set to NULL to skip verification (fine for dev, not for production).
#define HTTP_SERVER_CA_PEM  NULL

#define HTTP_TIMEOUT_MS  10000

// ─── Result type ──────────────────────────────────────────────────────────────
typedef struct {
    int  status_code;   // HTTP status, e.g. 200. -1 on connection/TLS error.
    bool success;       // true if status_code is 2xx
    char body[512];     // response body, null-terminated, truncated to fit
} http_result_t;

/**
 * POST a JSON string to HTTP_HOST + HTTP_PATH.
 *
 * @param json_payload  Null-terminated JSON string.
 * @return              http_result_t with the response.
 *
 * Ensure wifi_ensure_connected() returns true before calling this.
 */
http_result_t http_post_json(const char *json_payload);