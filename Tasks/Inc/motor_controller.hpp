#pragma once
#include "cmsis_os2.h"
#include "main.h"
#include "odrives1.hpp"

extern ODRIVES1 odrive_left;
extern ODRIVES1 odrive_right;

#define MAX_TORQUE 50.0
#define MIN_TORQUE -50.0
#define LOOP_PERIOD_MS 1 // 1 ms = l khz

void motorControllerInitTask();

// utility functions
float clamp(float max, float min, float value);
void delay_task_until(uint32_t &next_wake_time, float loop_period_ms);
