#include "motor_controller.hpp"
#include "stm32h7xx_hal_fdcan.h"
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

  while (odriveS1Handle == nullptr) {
    osDelay(10);
  }

  uint32_t time = HAL_GetTick();
  odriveS1Handle->getCANAddress();
  while(true)
  {
	  if (BSP_PB_GetState(BUTTON_USER) == BUTTON_PRESSED && HAL_GetTick() - time > 500) {
		  if (isClosedLoop) {
			  odriveS1Handle->setAxisState(AxisState::Idle);
			  isClosedLoop = false;
		  }
		  else {
			  odriveS1Handle->setAxisState(AxisState::ClosedLoopControl);
			  osDelay(100);
			  odriveS1Handle->setInputVelocity(-5, -0.5);
			  time = HAL_GetTick();
			  isClosedLoop = true;
			  BSP_LED_Off(LED_YELLOW);
		  }

		  time = HAL_GetTick();
	  }

	  const odrive_can_heartbeat_t heartbeat = odriveS1Handle->read(odriveS1Handle->heartbeat);
	  const odrive_can_bus_t bus = odriveS1Handle->read(odriveS1Handle->busVoltageCurrent);

	  if (heartbeat.axisState == static_cast<uint8_t>(AxisState::ClosedLoopControl)) {
		  BSP_LED_On(LED_RED);
	  }
	  else {
		  BSP_LED_Off(LED_RED);
	  }

	  if (bus.busCurrent > 0.1) {
		  odriveS1Handle->setAxisState(AxisState::Idle);
		  time = HAL_GetTick();
		  isClosedLoop = false;
		  BSP_LED_On(LED_YELLOW);
	  }

	  printf("Current: %f \r\n", bus.busCurrent);
	  BSP_LED_Toggle(LED_GREEN);
	  osDelay(50);
  }
}

void motorControllerInitTask()
{
    motorControllerTaskHandle = osThreadNew(motorControllerMainLoop, NULL, &motorController_attributes);
}
