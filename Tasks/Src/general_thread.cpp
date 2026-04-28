#include "general_thread.hpp"
#include <iostream>

#define MAX_TORQUE 50.0
#define MIN_TORQUE -50.0
#define LOOP_PERIOD_MS 1 // 1 ms = l khz

osThreadId_t taskHandle;

static const osThreadAttr_t thread_attributes = {
    .name = "general_thread",
    .stack_size = 2048,
    .priority = (osPriority_t) osPriorityNormal
};

struct MotorStates {
	float position;
	float velocity;
	float torque;
	float target_torque;
	float motor_temperature;
};

struct UI_Inputs {
	MotorStates left;
	MotorStates right;
	float battery_voltage;
	float battery_current;
};

static float clamp(float value, float max, float min) {
	if (value > max) {
		return max;
	}
	if (value < min) {
		return min;
	}
	return value;
}

static void delay_task_until(uint32_t &next_wake_time) {
    next_wake_time += LOOP_PERIOD_MS;
    int32_t delay_ms = (int32_t)(next_wake_time - osKernelGetTickCount());
    if (delay_ms > 0) {
        osDelayUntil(next_wake_time);
    } else {
        // missed deadline, reset to now
        next_wake_time = osKernelGetTickCount();
    	std::cout << "Control loop deadline missed!" << std::endl;
    }
}

void initTasks() {
  taskHandle = osThreadNew(mainLoop, NULL, &thread_attributes);
}

void mainLoop(void *arg) {
	ODRIVES1 odrive_left(&hfdcan1);
	ODRIVES1 odrive_right(&hfdcan1);

	ODRIVES1* odrives[2] = { &odrive_left, &odrive_right };
	MotorStates motor_states[2];
	odrive_can_encoder_estimates_t estimates[2];
	odrive_can_temperature_t temps[2];
	odrive_can_torque_t torques[2];
	odrive_can_bus_t bus_powers;

	ai_init();

	uint32_t next_wake_time = osKernelGetTickCount();

	while(true)
	{
	  // test LED blink
	  BSP_LED_Toggle(LED_GREEN);

	  // read left and right motor states
	  for(int i = 0; i < 2; i++) {
		  odrives[i]->getEncoderEstimates(&estimates[i]);
	      odrives[i]->getTemperatures(&temps[i]);
	      odrives[i]->getTorques(&torques[i]);
	      if (i == 0) {
	    	  // get the shared bus current and voltage
	    	  odrives[0]->getBusVoltageCurrent(&bus_powers);
	      }
	      motor_states[i].position = estimates[i].positionEstimate;
	      motor_states[i].velocity = estimates[i].velocityEstimate;
	      motor_states[i].motor_temperature = temps[i].motorTemperature;
	      motor_states[i].torque = torques[i].torqueEstimate;
	      motor_states[i].target_torque = torques[i].torqueTarget;
	  }

	  // send motor states to esp32, which will pass them to UI
	  UI_Inputs ui_inputs = {
		 motor_states[0],
		 motor_states[1],
		 bus_powers.busVoltage,
		 bus_powers.busCurrent
	  };

	  if (HAL_UART_GetState(&huart4) == HAL_UART_STATE_READY) {
		  HAL_StatusTypeDef res1 = HAL_UART_Transmit_IT(&huart4, (uint8_t*)&ui_inputs, sizeof(UI_Inputs));
		  if (res1 != HAL_OK) {
			  printf("UART1 transmission was not successful.\n");
		  }
	  }

	  // send motion data to RL policy
	  PolicyInputs ai_inputs = {
	    estimates[0].positionEstimate,
	    estimates[0].velocityEstimate,
	    estimates[1].positionEstimate,
	    estimates[1].velocityEstimate
	  };
	  PolicyOutputs outputs = {0};

	  // run DRL policy
	  if (ai_run(&ai_inputs, &outputs) != 0) {
		  // inference failed. safe fallback
		  outputs.left_torque = 0;
		  outputs.right_torque = 0;
	  }

	  // clamp torques to ensure they are within the safety range
	  float left_torque_filtered = clamp(outputs.left_torque, MAX_TORQUE, MIN_TORQUE);
	  float right_torque_filtered = clamp(outputs.right_torque, MAX_TORQUE, MIN_TORQUE);

	  // write the torques to the motors
	  odrive_left.setInputTorque(left_torque_filtered);
	  odrive_right.setInputTorque(right_torque_filtered);

	  // wait for next cycle
	  delay_task_until(next_wake_time);
	}
}
