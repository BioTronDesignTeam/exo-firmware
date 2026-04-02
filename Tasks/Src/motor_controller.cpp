#include "motor_controller.hpp"
#include "stm32H7xx_hal_fdcan.h"
#include "cmsis_os2.h"

osThreadId_t motorControllerTaskHandle;

static const osThreadAttr_t motorController_attributes = {
    .name = "MotorController",
    .stack_size = 1024,
    .priority = (osPriority_t) osPriorityNormal
};

// create motor driver instances only once
ODRIVES1 odrive_left(&hfdcan1);
ODRIVES1 odrive_right(&hfdcan1);

/*
 * Reads motor states and send it to UI and RL policy.
 */
void motorControllerMainLoop(void *arg)
{
  float left_torque = 0.0;
  float right_torque = 0.0;
  uint32_t next_wake_time = osKernelGetTickCount();

  while(true)
  {
    // TODO: receive resulting torques from RL policy

    // run safety check:
	// ensures torques are within the safety range
	left_torque = clamp(MAX_TORQUE, MIN_TORQUE, left_torque);
	right_torque = clamp(MAX_TORQUE, MIN_TORQUE, right_torque);

    // write the torques to the motors
	odrive_left.setInputTorque(left_torque);
	odrive_right.setInputTorque(right_torque);

	// wait until next cycle
	delay_task_until(next_wake_time, LOOP_PERIOD_MS);
  }
}

void motorControllerInitTask()
{
    motorControllerTaskHandle = osThreadNew(motorControllerMainLoop, NULL, &motorController_attributes);
}

float clamp(float max, float min, float value) {
	if (value > max) {
		return max;
	}
	if (value < min) {
		return min;
	}
	return value;
}

void delay_task_until(uint32_t &next_wake_time, float loop_period_ms) {
	next_wake_time += LOOP_PERIOD_MS;
	int32_t delay_ms = (int32_t)(next_wake_time - osKernelGetTickCount());
	if (delay_ms > 0) {
		// wait the remaining time
		osDelay(next_wake_time);
	} else {
		// reset
		next_wake_time = osKernelGetTickCount();
	}
}
