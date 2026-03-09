#include "imu.hpp"
#include "imu_controller.hpp"
#include "stm32H7xx_hal_fdcan.h"
#include <stdio.h>
#include <string.h>

extern I2C_HandleTypeDef hi2c2;
osThreadId_t imuTaskHandle;

static const osThreadAttr_t imu_attributes = {
    .name = "IMU",
    .stack_size = 1024,
    .priority = (osPriority_t) osPriorityNormal
};


void IMUMainLoop(void *arg)
{

	IMU imu(&hi2c2);
	imu.BNO_Init();
	imu.enableRotationVector(10000);
	while(true){
	   if(imu.dataAvailable()){
	       imu.parsePacket();
	       float qi = imu.getQuatI();
	    }
	}
}
void imuInitTask()
{
    imuTaskHandle = osThreadNew(IMUMainLoop, NULL, &imu_attributes);
}
