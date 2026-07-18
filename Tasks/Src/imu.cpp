#include "imu.hpp"
#include "mpu6050.hpp"
#include "drivers.hpp" 
#include <cmsis_os2.h>
#include <string.h>
extern "C" {
#include "main.h"
}

extern UART_HandleTypeDef huart4;
extern uint16_t crc16(uint8_t *data, uint32_t length); 


void read_imu_task(void *arg) {
    for ( ;; ) {
        if (MPU6050Handle != nullptr) {
            if (MPU6050Handle->getAll() != HAL_OK) {
                BSP_LED_On(LED_RED);
            }
        }
        osDelay(10); 
    }
}



mpu6050_data_t get_imu_data() {
    mpu6050_data_t data = {0};

    if (MPU6050Handle == nullptr) {
        return data;
    }

    data.accel_x = MPU6050Handle->mpu6050_acceleration.x;
    data.accel_y = MPU6050Handle->mpu6050_acceleration.y;
    data.accel_z = MPU6050Handle->mpu6050_acceleration.z;
    data.gyro_x  = MPU6050Handle->mpu6050_gyro.x;
    data.gyro_y  = MPU6050Handle->mpu6050_gyro.y;
    data.gyro_z  = MPU6050Handle->mpu6050_gyro.z;
    data.temperature = MPU6050Handle->mpu6050_temperature;

    return data;
}


void write_imu_to_esp(mpu6050_data_t data) {
    mpu6050_packet_t packet;
    packet.header[0] = 0xAA;
    packet.header[1] = 0x55;
    memcpy (&packet.data, &data, sizeof(data));

    packet.crc = crc16((uint8_t *)&packet.data, sizeof(data));

    HAL_StatusTypeDef err = HAL_UART_Transmit(&huart4, (uint8_t *)&packet, sizeof(mpu6050_packet_t), HAL_MAX_DELAY);
	
    if (err == HAL_OK) {
		BSP_LED_Toggle(LED_GREEN);
	}
	if (err == HAL_ERROR) {
		BSP_LED_Toggle(LED_RED);
	}
	if (err == HAL_TIMEOUT) {
		BSP_LED_Toggle(LED_YELLOW);
	}
	osDelay(500);
}

void send_imu_to_esp(void *arg) { //to esp32

    for ( ;; ) {
        mpu6050_data_t data = get_imu_data();
        write_imu_to_esp(data);
        osDelay(500);
    }
}

void init_imu() {
	osThreadId_t read_imu_task_handle;
	osThreadId_t send_imu_to_esp_handle;

    static const osThreadAttr_t read_imu_attributes = {
        .name = "ReadIMU",
        .stack_size = 1024,
        .priority = (osPriority_t) osPriorityBelowNormal7
    };
    static const osThreadAttr_t send_imu_attributes = {
        .name = "SendIMUtoESP",
        .stack_size = 1024,
        .priority = (osPriority_t) osPriorityBelowNormal7
    };
    read_imu_task_handle = osThreadNew(read_imu_task, NULL, &read_imu_attributes);
    send_imu_to_esp_handle = osThreadNew(send_imu_to_esp, NULL, &send_imu_attributes);
}
