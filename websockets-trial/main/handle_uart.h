#pragma once
#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint32_t example1;
    uint32_t example2;
    uint32_t example3;
} telemetry_data_t;
//uint32_t telemetry_data_size = 12; //3 uint32_t fields, update when adding to this


telemetry_data_t read_telemetry_from_uart();
void uart_init();
void uart2_mirror_task(void *arg);

