/**
 * uart_reader.c
 * ESP32 FreeRTOS — UART receive → WebSocket forwarder
 *
 * Responsibilities
 * ----------------
 * ESP32    : magic-byte sync (catches framing errors locally),
 *            packet accumulation, CRC-16 append, WebSocket send.
 * Receiver : CRC validation, field parsing.
 *
 * Wire format sent over WebSocket (binary):
 *   [0]        magic      : 0xAB  (sync word)
 *   [1..N]     payload    : raw UART bytes, excluding the \n delimiter
 *   [N+1..N+2] CRC-16 LE : CRC of bytes [1..N] only (payload only)
 *
 * Receiver workflow:
 *   1. Confirm byte[0] == 0xAB
 *   2. Extract payload  bytes[1..len-3]
 *   3. Compute CRC-16 over payload, compare with bytes[len-2..len-1]
 *   4. Parse payload fields if CRC matches
 *
 * Build with ESP-IDF (idf.py build).
 */
 //message sent:
 //first byte magic to sync
 //1...N payload excluding \n
 //n+1 and n+2 crc bytes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "driver/uart.h"
#include "esp_log.h"
#include "esp_timer.h"


#include "uart_consts.h"

static const char *TAG = "uart_fwd";


typedef struct {
    uint8_t  data[MAX_FRAME_LEN];
    uint16_t len;                  /* total bytes: magic + payload + crc */
    int64_t  timestamp_us;
} raw_frame_t;

//ring buffer

typedef struct {
    raw_frame_t       frames[RING_BUF_CAPACITY];
    volatile int      head;
    volatile int      tail;
    volatile int      count;
    SemaphoreHandle_t mutex;
} frame_ring_buf_t;

static frame_ring_buf_t g_ring;

static void ring_buf_init(frame_ring_buf_t *rb)
{
    memset(rb, 0, sizeof(*rb));
    rb->mutex = xSemaphoreCreateMutex();
    configASSERT(rb->mutex);
}

static bool ring_buf_push(frame_ring_buf_t *rb, const raw_frame_t *frame)
{
    if (xSemaphoreTake(rb->mutex, pdMS_TO_TICKS(10)) != pdTRUE) {
        return false;
    }

    rb->frames[rb->head] = *frame;
    rb->head = (rb->head + 1) % RING_BUF_CAPACITY;

    if (rb->count < RING_BUF_CAPACITY) {
        rb->count++;
    } else {
        //buffer full
        rb->tail = (rb->tail + 1) % RING_BUF_CAPACITY;
        ESP_LOGW(TAG, "Ring buffer full — oldest frame dropped");
    }

    xSemaphoreGive(rb->mutex);
    return true;
}

static bool ring_buf_pop(frame_ring_buf_t *rb, raw_frame_t *out)
{
    if (xSemaphoreTake(rb->mutex, pdMS_TO_TICKS(10)) != pdTRUE) {
        return false;
    }

    bool ok = false;
    if (rb->count > 0) {
        *out     = rb->frames[rb->tail];
        rb->tail = (rb->tail + 1) % RING_BUF_CAPACITY;
        rb->count--;
        ok = true;
    }

    xSemaphoreGive(rb->mutex);
    return ok;
}


//polynomial 0xA001
static uint16_t crc16(const uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFF;
    while (length--) {
        crc ^= *data++;
        for (int i = 0; i < 8; i++) {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

//two state machine: wait for magic byte, and parse payload bytes until \n
//once that happens crc bytes are appended and the frame is added to the ring buffer
//overflow drops the current frame since it's likely wrong

typedef enum { WAIT_MAGIC, ACCUMULATE } rx_state_t;

static void uart_rx_task(void *arg)
{
    uint8_t     raw[MAX_PAYLOAD_LEN];
    raw_frame_t frame;
    rx_state_t  state   = WAIT_MAGIC;
    int         pay_pos = 0;   /* write position within payload portion */

    ESP_LOGI(TAG, "UART RX task started");

    for (;;) {
        int len = uart_read_bytes(UART_WEBSOCKET_PORT, raw, sizeof(raw) - 1,
                                  pdMS_TO_TICKS(20));
        if (len <= 0) {
            taskYIELD();
            continue;
        }

        for (int i = 0; i < len; i++) {
            uint8_t b = raw[i];

            switch (state) {

            case WAIT_MAGIC:
                if (b == MAGIC_BYTE) {
                    memset(&frame, 0, sizeof(frame));
                    frame.data[0] = MAGIC_BYTE;
                    pay_pos       = 0;
                    state         = ACCUMULATE;
                }
                //until the magic byte is read everything is discarded
                break;

            case ACCUMULATE:
                if (b == '\n') {
                    if (pay_pos == 0) {
                        ESP_LOGW(TAG, "Empty payload, discarding");
                        state = WAIT_MAGIC;
                        break;
                    }

                    //crc ignores magic and end bytes
                    uint16_t crc = crc16(&frame.data[1], pay_pos);

                    
                    frame.data[1 + pay_pos]     = (uint8_t)(crc & 0xFF);
                    frame.data[1 + pay_pos + 1] = (uint8_t)(crc >> 8);
                    frame.len          = 1 + pay_pos + CRC_LEN;
                    frame.timestamp_us = esp_timer_get_time();

                    ring_buf_push(&g_ring, &frame);
                    ESP_LOGD(TAG, "Frame queued: %u bytes (payload=%d)",
                             frame.len, pay_pos);

                    state = WAIT_MAGIC;

                } else if (b == '\r') {
                    
                    //ignore this character

                } else {
                    if (pay_pos < MAX_PAYLOAD_LEN) {
                        frame.data[1 + pay_pos] = b;
                        pay_pos++;
                    } else {
                        //ignore overload
                        ESP_LOGW(TAG, "Payload overflow, dropping frame and waiting for next magic");
                        state = WAIT_MAGIC;
                    }
                }
                break;
            }
        }
    }
}


static void uart_to_websocket_forwarding(void *arg)
{
    raw_frame_t frame;

    ESP_LOGI(TAG, "websocketforwarding task started");

    for (;;) {
        if (ring_buf_pop(&g_ring, &frame)) {
            
            ESP_LOGI(TAG, "WS send: %u bytes (ts=%" PRId64 " us)",
                     frame.len, frame.timestamp_us);
            websocket_send(frame)
        } else {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}


static void uart_init(void)
{
    const uart_config_t cfg = {
        .baud_rate  = UART_BAUD,
        .data_bits  = UART_DATA_8_BITS,
        .parity     = UART_PARITY_DISABLE,
        .stop_bits  = UART_STOP_BITS_1,
        .flow_ctrl  = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_param_config(UART_WEBSOCKET_PORT, &cfg));
    ESP_ERROR_CHECK(uart_set_pin(UART_WEBSOCKET_PORT, UART_TX_PIN, UART_RX_PIN,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_WEBSOCKET_PORT, UART_BUF_SIZE * 2,
                                        0, 0, NULL, 0));

    ESP_LOGI(TAG, "UART%d initialised @ %d baud (TX=%d RX=%d)",
             UART_WEBSOCKET_PORT, UART_BAUD, UART_TX_PIN, UART_RX_PIN);
}
