#pragma once

#include <cstdint>

enum class SafetyTask : uint8_t {
    MotorControl,
    Count
};

bool safety_init();
void safety_register(SafetyTask task, uint32_t deadlineMs);
void safety_checkin(SafetyTask task);
bool safety_tripped();
