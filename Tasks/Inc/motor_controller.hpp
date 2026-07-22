#pragma once
#include "cmsis_os2.h"
#include "drivers.hpp"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo.h"

typedef struct __attribute__((packed)) {
    float position;
    float velocity;
    float busVoltage;
    float busCurrent;
    uint8_t axisState;
    uint32_t activeErrors;
} telemetry_data_t;

void motorControllerInitTask();
telemetry_data_t get_motor_telemetry();
