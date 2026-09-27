#include "msa311.hpp"
#include "msa311_registers.hpp"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo.h"
#include "uart.hpp"
#include "cstring"

MSA311::MSA311(I2C_HandleTypeDef* i2cHandle) : _i2c(i2cHandle) {
    uint8_t data = 0x00;
    HAL_I2C_Mem_Write(_i2c, MSA311_I2C_ADDR, MSA311_SOFT_RESET, I2C_MEMADD_SIZE_8BIT, &data, 1, 100);
    HAL_Delay(10); 
    uint8_t partid = 0;
    HAL_I2C_Mem_Read(_i2c, MSA311_I2C_ADDR, MSA311_PARTID, I2C_MEMADD_SIZE_8BIT, &partid, 1, 100);
    serial_print_byte(partid);
    // Turn on yellow LED if communication fails
    if (partid != MSA311_PARTID_VAL) {
    	//BSP_LED_On(LED_YELLOW);
    }
}

HAL_StatusTypeDef MSA311::getAccel() {
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(_i2c, MSA311_I2C_ADDR, MSA311_ACC_X_LSB, I2C_MEMADD_SIZE_8BIT, rxBuffer, 6, 100);
    if (status != HAL_OK) return status; 

    int16_t rawX = (int16_t)(rxBuffer[1] << 8 | rxBuffer[0]);
    int16_t rawY = (int16_t)(rxBuffer[3] << 8 | rxBuffer[2]);
    int16_t rawZ = (int16_t)(rxBuffer[5] << 8 | rxBuffer[4]);

    msa311_data.x = rawX / _accelSensitivity;
    msa311_data.y = rawY / _accelSensitivity;
    msa311_data.z = rawZ / _accelSensitivity;

    return status;
}

HAL_StatusTypeDef MSA311::setAccelRange(MSA311Range range) {
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(_i2c, MSA311_I2C_ADDR, MSA311_RANGE, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&range, 1, 100);
    switch (range) {
        case MSA311Range::RANGE_2G:
            _accelSensitivity = 16384.0f;
            break;
        case MSA311Range::RANGE_4G:
            _accelSensitivity = 8192.0f;
            break;
        case MSA311Range::RANGE_8G:
            _accelSensitivity = 4096.0f;
            break;
        case MSA311Range::RANGE_16G:
            _accelSensitivity = 2048.0f;
            break;
    }
    return status;
}

HAL_StatusTypeDef MSA311::setODR(MSA311ODR odr) {
    return HAL_I2C_Mem_Write(_i2c, MSA311_I2C_ADDR, MSA311_ODR, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&odr, 1, 100);
}

HAL_StatusTypeDef MSA311::setPowerMode(MSA311PowerMode mode) {
    return HAL_I2C_Mem_Write(_i2c, MSA311_I2C_ADDR, MSA311_POWER_MODE, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&mode, 1, 100);
}
