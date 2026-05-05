#pragma once


static void ring_buf_init(frame_ring_buf_t *rb);
static void uart_rx_task(void *arg);
static void uart_to_websocket_forwarding(void *arg);
static void uart_init(void);
