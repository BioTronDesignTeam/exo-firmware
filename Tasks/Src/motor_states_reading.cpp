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

struct MotorStates {
	float position;
	float velocity;
	float torque;
	float target_torque;
	float FET_temperature;
	float motor_temperature;
};

/*
 * Reads motor states and send it to UI and RL policy.
 */
void motorStatesReadingMainLoop(void *arg)
{
  
  ODRIVES1 odrive_left(&hfdcan1);
  ODRIVES1 odrive_right(&hfdcan2);

  ODRIVES1* odrives[2] = { &odrive_left, &odrive_right };
  MotorStates motor_states[2];
  odrive_can_encoder_estimates_t estimates[2];
  odrive_can_temperature_t temps[2];
  odrive_can_torque_t torques[2];
  odrive_can_power_t powers[2];

  // motor states that will be sent to UI
  MotorStates left_states, right_states;
  // buffer storing all motor states
  uint8_t uart_buffer1[sizeof(MotorStates) * 2];
  // buffer storing motion data for RL (left & right hip motor pos and vel)
  uint8_t uart_buffer2[sizeof(float) * 4];

  uint32_t next_wake_time = osKernelGetTickCount();

  while(true)
  {
    BSP_LED_Toggle(LED_GREEN);

    // read left and right motor states
    for(int i = 0; i < 2; i++) {
        odrives[i]->getEncoderEstimates(&estimates[i]);
        odrives[i]->getTemperatures(&temps[i]);
        odrives[i]->getTorques(&torques[i]);
        odrives[i]->getPowers(&powers[i]);

        motor_states[i].position = estimates[i].positionEstimate;
        motor_states[i].velocity = estimates[i].velocityEstimate;
        motor_states[i].FET_temperature = temps[i].FETTemperature;
        motor_states[i].motor_temperature = temps[i].motorTemperature;
        motor_states[i].torque = torques[i].torqueEstimate;
        motor_states[i].target_torque = torques[i].torqueTarget;
    }

    // send motor states to esp32, which will pass it to UI
    HAL_StatusTypeDef res1 = HAL_UART_Transmit(&huart1, (uint8_t*)motor_states, sizeof(motor_states), 5);
    if (res1 != HAL_OK) {
    	printf("UART 1 transmission was not successful.\n");
    }

    // send motion data to RL policy
    float rl_data[4] = {
        estimates[0].positionEstimate,
        estimates[0].velocityEstimate,
        estimates[1].positionEstimate,
        estimates[1].velocityEstimate
    };
    HAL_StatusTypeDef res2 = HAL_UART_Transmit(&huart2, (uint8_t*)rl_data, sizeof(rl_data), 5);
    if (res2 != HAL_OK) {
    	printf("UART 2 transmission was not successful.\n");
    }

    delay_task_until(next_wake_time, LOOP_PERIOD_MS);
  }
}

void  motorStatesReadingInitTask()
{
	motorStatesReadingTaskHandle = osThreadNew(motorStatesReadingMainLoop, NULL, &motorStatesReading_attributes);
}
