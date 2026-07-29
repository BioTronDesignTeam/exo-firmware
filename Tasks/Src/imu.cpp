#include "imu.hpp"
#include "drivers.hpp"
#include "uart.hpp"
#include <cmsis_os2.h>
#include <string.h>
#include <stdio.h>

extern "C" {
#include "main.h"
}

void update_msa311(void *arg) {
    char buf[64];
    for ( ;; ) {
        //esp_print((uint8_t*)"Updating MPU6050 lol wrong imu\r\n", 19);
        ESP_PRINT("Updating MPU6050 lol wrong imu\r\n");
        BSP_LED_Toggle(LED_YELLOW);

        if (MSA311Handle != nullptr) {
            if (MSA311Handle->getAccel() == HAL_OK) {
                snprintf(buf, sizeof(buf), "MSA X=%.2f Y=%.2f Z=%.2f\r\n",
                         MSA311Handle->msa311_data.x,
                         MSA311Handle->msa311_data.y,
                         MSA311Handle->msa311_data.z);
                ESP_PRINT(buf);
            } else {
                ESP_PRINT("MSA read failed\r\n");
            }
        }
        osDelay(500);
    }
}


void init_imu_tasks() {
    osThreadId_t updateMSA311Handle;

    static const osThreadAttr_t updateMSA311Attributes = {
        .name = "UpdateMSA311",
        .stack_size = 1024,
        .priority = (osPriority_t) osPriorityNormal
    };
    //esp_print((uint8_t*)"IMU Tasks Initialized\r\n", 23);
    updateMSA311Handle = osThreadNew(update_msa311, NULL, &updateMSA311Attributes);
}
