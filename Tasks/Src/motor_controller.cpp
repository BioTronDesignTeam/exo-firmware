#include "motor_controller.hpp"

osThreadId_t motorControllerTaskHandle;

static const osThreadAttr_t motorController_attributes = {
    .name = "MotorController",
    .stack_size = 1024,
    .priority = (osPriority_t) osPriorityNormal
};

void motorControllerMainLoop(void *arg)
{

  while(true)
  {
    BSP_LED_Toggle(LED_GREEN);
    osDelay(1000);
  }
}

void motorControllerInitTask()
{
    motorControllerTaskHandle = osThreadNew(motorControllerMainLoop, NULL, &motorController_attributes);
}
