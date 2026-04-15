#include "motor_controller.hpp"
#include "stm32H7xx_hal_fdcan.h"

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
			  isClosedLoop = true;
		  }
	  }
	  if (odriveS1Handle->heartbeat.axisState == 0x8) {
		  BSP_LED_On(LED_RED);
	  }
	  else {
		  BSP_LED_Off(LED_RED);
	  }
	  BSP_LED_Toggle(LED_GREEN);
	  HAL_Delay(500);
  }
}

void motorControllerInitTask()
{
    motorControllerTaskHandle = osThreadNew(motorControllerMainLoop, NULL, &motorController_attributes);
}
