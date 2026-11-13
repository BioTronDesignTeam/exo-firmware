#pragma once

#include <cstddef>
#include <cstdint>

extern "C" 
{
#include "main.h"
#include "stm32h7xx_hal_def.h"
}

struct __attribute__((packed)) serial_telemetry_payload_t 
{
    uint32_t timestamp_ms;
    float accel_x_g;
    float accel_y_g;
    float accel_z_g;
    float bno_quaternion_i;
    float bno_quaternion_j;
    float bno_quaternion_k;
    float bno_quaternion_real;
    float bno_accuracy_radians;
    uint8_t bno_status;
    uint8_t bno_valid;
}; // 38 bytes

// bool serial_protocol_init(); // called locally
// bool send_serial_packet(SerialPacketType type, const void* payload, uint16_t length); // called locally
// bool send_serial_log(const char* message, uint16_t length); // called locally

#define SERIAL_PRINT(s) serial_print((uint8_t*)(s), (uint16_t)strlen(s)) // called in imu.cpp
#define SERIAL_PRINTLN(s) serial_print((uint8_t*)(s "\r\n"), (uint16_t)strlen(s "\r\n")) // not called?

HAL_StatusTypeDef serial_print_byte(uint8_t byte); // called by msa311.cpp (deprecated?)
HAL_StatusTypeDef serial_print(uint8_t* str, uint16_t len); // called locally
bool send_serial_telemetry(const serial_telemetry_payload_t& telemetry); // for imus to call?
void init_uart_tasks(); // called by general_thread.cpp
