#pragma once
#include <stdint.h>

typedef struct __attribute__((packed)) { //UPDATE THESE IN THE ESP FOLDER AS WELL!!
    uint32_t example1;
    uint32_t example2;
    uint32_t example3;
} telemetry_data_t;

typedef struct __attribute__((packed)) {
    uint8_t header[2]; //0xAA, 0x55
    uint16_t data_length; //length of the data in bytes
    telemetry_data_t data;
    uint16_t crc; //crc16 of the data
} telemetry_packet_t;

void spamUART(void* arg);
void init_uart_tasks();


