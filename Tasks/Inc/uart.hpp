#pragma once
#include <stdint.h>
#include "mpu6050_data.hpp"
#include <string.h>
extern "C" {

#include "main.h"
#include "stm32h7xx_hal_def.h"

}
#define ESP_PRINT(s) esp_print((uint8_t*)(s), (uint16_t)strlen(s))
#define ESP_PRINTLN(s) esp_print((uint8_t*)(s "\r\n"), (uint16_t)strlen(s "\r\n"))

typedef struct __attribute__((packed)) { //UPDATE THESE IN THE ESP FOLDER AS WELL!!
    uint32_t example1;
    uint32_t example2;
    uint32_t example3;
    mpu6050_data_t mpu6050_data;
} telemetry_data_t;

typedef struct __attribute__((packed)) {
    uint8_t header[2]; //0xAA, 0x55
    telemetry_data_t data;
    uint16_t crc; //crc16 of the data
} telemetry_packet_t;

HAL_StatusTypeDef esp_print_byte(uint8_t byte);
HAL_StatusTypeDef esp_print(uint8_t* str, uint16_t len);
void spamUART(void* arg);
void init_uart_tasks();


