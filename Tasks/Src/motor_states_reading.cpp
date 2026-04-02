#include "motor_controller.hpp"
#include "odrives1.hpp"
#include "stm32H7xx_hal_fdcan.h"

extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_HandleTypeDef hfdcan2;

osThreadId_t motorStatesReadingTaskHandle;

static const osThreadAttr_t motorStatesReading_attributes = {
    .name = "MotorStatesReading",
    .stack_size = 1024,
    .priority = (osPriority_t) osPriorityNormal
};

/*
 * Reads motor states and send it to UI and RL policy.
 */
void motorStatesReadingMainLoop(void *arg)
{
  
  ODRIVES1 odrive_left(&hfdcan1);
  ODRIVES1 odrive_right(&hfdcan2);

  odrive_can_heartbeat_t heartbeat = {0};

  odrive_can_encoder_estimates_t left_estimates = {0, 0};
  odrive_can_temperature_t left_temperature = {0, 0};
  odrive_can_torque_t left_torque = {0, 0};

  odrive_can_encoder_estimates_t right_estimates = {0, 0};
  odrive_can_temperature_t right_temperature = {0, 0};
  odrive_can_torque_t right_torque = {0, 0};
  odrive_can_power_t power = {0, 0};

  uint32_t next_wake_time = osKernelGetTickCount();

  while(true)
  {
    BSP_LED_Toggle(LED_GREEN);

    // receive left motor states
    odrive_left.getEncoderEstimates(&left_estimates);
    odrive_left.getTemperatures(&left_temperature);
    odrive_left.getTorques(&left_torque);
    odrive_left.getPowers(&power);

    // receive right motor states
    odrive_right.getEncoderEstimates(&right_estimates);
    odrive_right.getTemperatures(&right_temperature);
    odrive_right.getTorques(&right_torque);

    // TODO: send motor states to UI

    // TODO: send motion data to RL policy

    delay_task_until(next_wake_time, LOOP_PERIOD_MS);
  }
}

void  motorStatesReadingInitTask()
{
	motorStatesReadingTaskHandle = osThreadNew(motorStatesReadingMainLoop, NULL, &motorStatesReading_attributes);
}
