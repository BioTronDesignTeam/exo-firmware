#include "motor_controller.hpp"
#include "odrives1.hpp"
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
  
  ODRIVES1 odrive(&hfdcan1);
  odrive_can_heartbeat_t heartbeat = {0};

  while(true)
  {
    BSP_LED_Toggle(LED_GREEN);
    //odrive.getHeartbeat(&heartbeat);
    osDelay(1000);
  }
}

void motorControllerInitTask()
{
    motorControllerTaskHandle = osThreadNew(motorControllerMainLoop, NULL, &motorController_attributes);
}
