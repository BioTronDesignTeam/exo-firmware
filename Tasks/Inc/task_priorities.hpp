#pragma once

#include "cmsis_os2.h"

namespace TaskPriority {

constexpr osPriority_t Startup = osPriorityRealtime7;
constexpr osPriority_t MotorControl = osPriorityHigh;
constexpr osPriority_t Imu = osPriorityAboveNormal;
constexpr osPriority_t SerialRx = osPriorityNormal;
constexpr osPriority_t SerialTx = osPriorityNormal;
constexpr osPriority_t DriverInit = osPriorityNormal;
constexpr osPriority_t Telemetry = osPriorityBelowNormal;
constexpr osPriority_t Diagnostics = osPriorityLow;

}
