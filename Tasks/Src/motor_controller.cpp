#include "motor_controller.hpp"
#include "stm32H7xx_hal_fdcan.h"
#include <stdio.h>

extern FDCAN_HandleTypeDef hfdcan1;

osThreadId_t motorControllerTaskHandle;

static const osThreadAttr_t motorController_attributes = {
    .name = "MotorController",
    .stack_size = 1024,
    .priority = (osPriority_t) osPriorityNormal
};

void motorControllerMainLoop(void *arg)
{
  bool isClosedLoop = false;

  uint32_t time = HAL_GetTick();
  odriveS1Handle->getCANAddress();
  while(true)
  {
	  if (BSP_PB_GetState(BUTTON_USER) == BUTTON_PRESSED && HAL_GetTick() - time > 500) {
		  if (isClosedLoop) {
			  odriveS1Handle->setAxisState(0x1);
			  isClosedLoop = false;
		  }
		  else {
			  odriveS1Handle->setAxisState(0x8);
			  HAL_Delay(100);
			  odriveS1Handle->setInputVelocity(-5, -0.5);
			  time = HAL_GetTick();
			  isClosedLoop = true;
			  BSP_LED_Off(LED_YELLOW);
		  }

		  time = HAL_GetTick();
	  }

	  if (odriveS1Handle->heartbeat.axisState == 0x8) {
		  BSP_LED_On(LED_RED);
	  }
	  else {
		  BSP_LED_Off(LED_RED);
	  }

	  if (odriveS1Handle->busVoltageCurrent.busCurrent > 0.1) {
		  odriveS1Handle->setAxisState(0x1);
		  time = HAL_GetTick();
		  isClosedLoop = false;
		  BSP_LED_On(LED_YELLOW);
	  }

	  printf("Current: %f \r\n", odriveS1Handle->busVoltageCurrent.busCurrent);
	  BSP_LED_Toggle(LED_GREEN);
	  HAL_Delay(50);
  }
}

void motorControllerInitTask()
{
    motorControllerTaskHandle = osThreadNew(motorControllerMainLoop, NULL, &motorController_attributes);
}

telemetry_data_t get_motor_telemetry()
{
	telemetry_data_t data = {0};

	if (odriveS1Handle == nullptr) {
		return data;
	}

	data.position = odriveS1Handle->encoderEstimates.positionEstimate;
	data.velocity = odriveS1Handle->encoderEstimates.velocityEstimate;
	data.busVoltage = odriveS1Handle->busVoltageCurrent.busVoltage;
	data.busCurrent = odriveS1Handle->busVoltageCurrent.busCurrent;
	data.axisState = odriveS1Handle->heartbeat.axisState;
	data.activeErrors = odriveS1Handle->error.activeErrors;

	return data;
}
