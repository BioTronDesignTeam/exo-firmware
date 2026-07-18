#pragma once

/*
    * I2C driver for the MPU6050 IMU
    * Author: Aria Xiong
*/
#ifndef INC_MPU6050_I2C_HPP_
#define INC_MPU6050_I2C_HPP_

#include "stm32H7xx_hal.h"
#include "mpu6050_registers.hpp"
#include "mpu6050_data.hpp"
class MPU6050{
private: 
    I2C_HandleTypeDef* _i2c;
    uint8_t rxBuffer[14] = {0}; 

    float _gyroBiasX = 0.0f;
    float _gyroBiasY = 0.0f;
    float _gyroBiasZ = 0.0f;

    float _accelSensitivity = 16384.0f; //default 2g
    float _gyroSensitivity = 131.0f; //default 250deg/s

public:

    // Sensor data structs
    mpu6050_data_t mpu6050_data = {0};


    // Constructor stores the I2C handle. It also wakes up the MPU6050 and verifies communication by reading the WHO_AM_I register.
    // Turns on the red LED if communication fails.
    MPU6050(I2C_HandleTypeDef* i2cHandle);

    // Reads all sensor data (accel + gyro + temp) in one transaction
    // return HAL_OK on success, HAL_ERROR on failure
    HAL_StatusTypeDef getAll();

    // Reads just the accelerometer data
    HAL_StatusTypeDef getAccel();

    // Reads the gyroscope data
    HAL_StatusTypeDef getGyro();
    
    // Reads the temperature data
    HAL_StatusTypeDef getTemp();

    /**Configuration functions */
    // Sets accelerometer full scale range and updates sensitivity accordingly
    // param range AccelRange::G2, G4, G8, or G16
    HAL_StatusTypeDef setAccelRange(AccelRange range);

    // Sets gyroscope full scale range and updates sensitivity accordingly
    // param range GyroRange::DPS250, DPS500, DPS1000, or DPS2000
    HAL_StatusTypeDef setGyroRange(GyroRange range);

    // Sets the sample rate divider
    HAL_StatusTypeDef setSampleRate(uint8_t divider);

    // Calibrates gyroscope bias by averaging 500 samples at stationary.
    // Bias is subtracted from getGyro and getAll readings.
    HAL_StatusTypeDef calibrateGyro();
    
};
#endif
