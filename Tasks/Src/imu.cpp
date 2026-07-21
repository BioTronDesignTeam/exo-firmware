#include "imu.hpp"
#include "drivers.hpp"
#include "uart.hpp"
#include <cmsis_os2.h>
#include <string.h>

extern "C" {
#include "main.h"
}

void update_mpu6050(void *arg) {
    for ( ;; ) {
        esp_print((uint8_t*)"Updating MPU6050\r\n", 19);
        BSP_LED_Toggle(LED_YELLOW);
//        if (MPU6050Handle != nullptr) {
//            if (MPU6050Handle->getAll() != HAL_OK) {
//                //BSP_LED_Toggle(LED_RED);
//            }
//        }
        osDelay(500);
    }
}


void init_imu_tasks() {
    osThreadId_t updateMPU6050Handle;

    static const osThreadAttr_t updateMPU6050Attributes = {
        .name = "UpdateMPU6050",
        .stack_size = 1024,
        .priority = (osPriority_t) osPriorityNormal
    };
    esp_print((uint8_t*)"IMU Tasks Initialized\r\n", 23);
    updateMPU6050Handle = osThreadNew(update_mpu6050, NULL, &updateMPU6050Attributes);
}
