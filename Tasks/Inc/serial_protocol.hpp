#pragma once

#include <cstdint>
#include <cstring>

enum class SerialPacketType : uint8_t 
{
    Telemetry = 0x01,
    LogMessage = 0x04,
    Ping = 0x05,
    Ack = 0x06,
};

bool init_serial_tasks(); 
bool queue_serial_packet(SerialPacketType type, const void* payload, uint16_t length); 

#define SERIAL_PRINT(s) queue_serial_packet(SerialPacketType::LogMessage, (s), (uint16_t)strlen(s))
#define SERIAL_PRINTLN(s) queue_serial_packet(SerialPacketType::LogMessage, (s "\r\n"), (uint16_t)strlen(s "\r\n"))
