#pragma once
#include <stdint.h>

typedef struct __attribute__((packed)) { //UPDATE THESE IN THE STM FOLDER AS WELL!!
    uint32_t example1;
    uint32_t example2;
    uint32_t example3;
} telemetry_data_t;

typedef struct __attribute__((packed)) {
    uint8_t header[2]; //0xAA, 0x55
    telemetry_data_t data;
    uint16_t crc; //crc16 of the data
} telemetry_packet_t;

void read_telemetry_from_uart();
void write_data_to_stm (telemetry_data_t data);
void uart_init();
void uart2_mirror_task(void *arg) ;

