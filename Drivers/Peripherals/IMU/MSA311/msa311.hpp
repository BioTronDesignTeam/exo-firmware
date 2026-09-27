#pragma once

#include "stm32h7xx_hal.h"
#include "msa311_registers.hpp"

class MSA311 {
private:
    I2C_HandleTypeDef* _i2c;
    uint8_t rxBuffer[6] = {0};

    float _accelSensitivity = 16384.0f; //default 2g

public:
    // Sensor data structs
    struct {
        float x, y, z; // units: g
    } msa311_data = {0};

    // Constructor stores the I2C handle. It also wakes up the MSA311 and verifies communication by reading the PARTID register.
    // Turns on the red LED if communication fails.
    MSA311(I2C_HandleTypeDef* i2cHandle);

    // Reads all sensor data (accel) in one transaction
    // return HAL_OK on success, HAL_ERROR on failure
    HAL_StatusTypeDef getAccel();

    /**Configuration functions */
    // Sets accelerometer full scale range and updates sensitivity accordingly
    // param range MSA311Range::RANGE_2G, RANGE_4G, RANGE_8G, or RANGE_16G
    HAL_StatusTypeDef setAccelRange(MSA311Range range);

    // Sets output data rate
    HAL_StatusTypeDef setODR(MSA311ODR odr);

    // Sets power mode
    HAL_StatusTypeDef setPowerMode(MSA311PowerMode mode);
};

