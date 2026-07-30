#pragma once

#include <cstddef>
#include <cstdint>

enum class SerialPacketType : uint8_t {
    Telemetry = 0x01,
    LogMessage = 0x04,
    Ping = 0x05,
    Ack = 0x06,
};

constexpr uint16_t SERIAL_PROTOCOL_MAX_PAYLOAD = 256;

struct __attribute__((packed)) serial_telemetry_payload_t {
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
};

bool serial_protocol_init();
bool send_serial_packet(SerialPacketType type, const void* payload, uint16_t length);
bool send_serial_log(const char* message, uint16_t length);
