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
	  if (BSP_PB_GetState(BUTTON_USER) == BUTTON_PRESSED) {
		  if (isClosedLoop) {
			  odriveS1Handle->setAxisState(0x1);
			  isClosedLoop = false;
		  }
		  else {
			  odriveS1Handle->setAxisState(0x8);
			  HAL_Delay(500);
			  odriveS1Handle->setInputVelocity(2, 1);
			  time = HAL_GetTick();
			  isClosedLoop = true;
			  BSP_LED_Off(LED_YELLOW);
		  }
	  }
	  if (odriveS1Handle->heartbeat.axisState == 0x8) {
		  BSP_LED_On(LED_RED);
	  }
	  else {
		  BSP_LED_Off(LED_RED);
	  }
	  if (odriveS1Handle->iq.iqMeasured > 0.1 && time - HAL_GetTick() > 1000) {
		  odriveS1Handle->setAxisState(0x1);
		  isClosedLoop = false;
		  BSP_LED_On(LED_YELLOW);
	  }
	  printf("Heartbeat Axis State: %d \r\n", odriveS1Handle->heartbeat.axisState);
	  printf("Heartbeat Axis Error: %ld \r\n", odriveS1Handle->heartbeat.axisError);
	  printf("IQ Measured: %f \r\n", odriveS1Handle->iq.iqMeasured);
	  BSP_LED_Toggle(LED_GREEN);
	  HAL_Delay(500);
  }
}

void motorControllerInitTask()
{
    motorControllerTaskHandle = osThreadNew(motorControllerMainLoop, NULL, &motorController_attributes);
}
