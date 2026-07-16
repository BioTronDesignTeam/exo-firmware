#pragma once
#include <stdint.h>

typedef struct __attribute__((packed)) { 
    float accel_x; // g
    float accel_y;
    float accel_z;
    float gyro_x;  // deg/s
    float gyro_y;
    float gyro_z;
    float temperature; // celsius
} mpu6050_data_t;

typedef struct __attribute__((packed)) {
    uint8_t header[2]; //0xAA, 0x55
    mpu6050_data_t data;
    uint16_t crc; //crc16 of data
} mpu6050_packet_t;

void init_imu();
