#pragma once
#include "uart_consts.h"


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

extern frame_ring_buf_t g_ring;

void ring_buf_init(frame_ring_buf_t *rb);
void uart_rx_task(void *arg);
void uart_to_websocket_forwarding(void *arg);
void uart_init(void);
