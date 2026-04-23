#pragma once
#include "odriveS1.hpp"
#include "ai_inference.h"
#include "stm32h7xx_hal.h"
#include "cmsis_os2.h"


static float clamp(float value, float max, float min);
static void delay_task_until(uint32_t &next_wake_time);

void initTasks();
void mainLoop(void *arg);
