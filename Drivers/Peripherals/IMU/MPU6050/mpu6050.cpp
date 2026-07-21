#include "mpu6050.hpp"

#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo.h"
#include "mpu6050_registers.hpp"

MPU6050::MPU6050(I2C_HandleTypeDef* i2cHandle):_i2c(i2cHandle){
    // Wake up the MPU6050 by writing 0 to the PWR_MGMT_1 register
    uint8_t data = 0x00;
    HAL_I2C_Mem_Write(_i2c,MPU6050_I2C_ADDR,PWR_MGMT_1,I2C_MEMADD_SIZE_8BIT,&data,
        1,HAL_MAX_DELAY);
    // Verify communication by reading the WHO_AM_I register
    uint8_t id = 0;
    HAL_I2C_Mem_Read(_i2c, MPU6050_I2C_ADDR, WHO_AM_I, I2C_MEMADD_SIZE_8BIT, &id,
        1, HAL_MAX_DELAY);
    if(id != MPU6050_WHO_AM_I_VAL){
        //BSP_LED_On(LED_);
    }
}

HAL_StatusTypeDef MPU6050::getAll(){
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(_i2c, MPU6050_I2C_ADDR, ACCEL_XOUT_H, I2C_MEMADD_SIZE_8BIT, rxBuffer, 14, HAL_MAX_DELAY);
    if (status != HAL_OK) return status; // don't parse a failed/partial read

    int16_t accelx = (int16_t)(rxBuffer[0] << 8 | rxBuffer[1]);
    int16_t accely = (int16_t)(rxBuffer[2] << 8 | rxBuffer[3]);
    int16_t accelz = (int16_t)(rxBuffer[4] << 8 | rxBuffer[5]);
    mpu6050_data.accel_x = accelx/_accelSensitivity;
    mpu6050_data.accel_y = accely/_accelSensitivity;
    mpu6050_data.accel_z = accelz/_accelSensitivity;

    int16_t rawTemp = (int16_t)(rxBuffer[6] << 8 | rxBuffer[7]);
    mpu6050_data.temp =  rawTemp/340.0f + 36.53f;

    int16_t gyrox  = (int16_t)(rxBuffer[8]  << 8 | rxBuffer[9]);
    int16_t gyroy  = (int16_t)(rxBuffer[10] << 8 | rxBuffer[11]);
    int16_t gyroz  = (int16_t)(rxBuffer[12] << 8 | rxBuffer[13]);
    mpu6050_data.gyro_x = gyrox / _gyroSensitivity - _gyroBiasX;
    mpu6050_data.gyro_y = gyroy / _gyroSensitivity - _gyroBiasY;
    mpu6050_data.gyro_z = gyroz / _gyroSensitivity - _gyroBiasZ;

    return status;
}

HAL_StatusTypeDef MPU6050::getAccel(){
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(_i2c, MPU6050_I2C_ADDR, ACCEL_XOUT_H, I2C_MEMADD_SIZE_8BIT, rxBuffer, 6, HAL_MAX_DELAY);
    
    int16_t accelx = (int16_t)(rxBuffer[0] << 8 | rxBuffer[1]);
    int16_t accely = (int16_t)(rxBuffer[2] << 8 | rxBuffer[3]);
    int16_t accelz = (int16_t)(rxBuffer[4] << 8 | rxBuffer[5]);
    mpu6050_data.accel_x = accelx/_accelSensitivity;
    mpu6050_data.accel_y = accely/_accelSensitivity;
    mpu6050_data.accel_z = accelz/_accelSensitivity;

    return status;
}

HAL_StatusTypeDef MPU6050::getGyro(){
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(_i2c, MPU6050_I2C_ADDR, GYRO_XOUT_H, I2C_MEMADD_SIZE_8BIT, rxBuffer, 6, HAL_MAX_DELAY);
    
    int16_t gyrox  = (int16_t)(rxBuffer[0]  << 8 | rxBuffer[1]);
    int16_t gyroy  = (int16_t)(rxBuffer[2] << 8 | rxBuffer[3]);
    int16_t gyroz  = (int16_t)(rxBuffer[4] << 8 | rxBuffer[5]);
    mpu6050_data.gyro_x = gyrox / _gyroSensitivity - _gyroBiasX;
    mpu6050_data.gyro_y = gyroy / _gyroSensitivity - _gyroBiasY;
    mpu6050_data.gyro_z = gyroz / _gyroSensitivity - _gyroBiasZ;

    return status;
}

HAL_StatusTypeDef MPU6050::getTemp(){
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(_i2c, MPU6050_I2C_ADDR, TEMP_OUT_H, I2C_MEMADD_SIZE_8BIT, rxBuffer, 2, HAL_MAX_DELAY);
    
    int16_t rawTemp = (int16_t)(rxBuffer[0] << 8 | rxBuffer[1]);
    mpu6050_data.temp =  rawTemp/340.0f + 36.53f;

    return status;
}

HAL_StatusTypeDef MPU6050::setAccelRange(AccelRange range){
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(_i2c, MPU6050_I2C_ADDR, ACCEL_CONFIG, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&range, 1, HAL_MAX_DELAY);
    switch (range) {
        case AccelRange::G2:
            _accelSensitivity = 16384.0f;
            break;
        case AccelRange::G4:
            _accelSensitivity = 8192.0f;
            break;
        case AccelRange::G8:
            _accelSensitivity = 4096.0f;
            break;
        case AccelRange::G16:
            _accelSensitivity = 2048.0f;
            break;
    }
    
    return status;
}

HAL_StatusTypeDef MPU6050::setGyroRange(GyroRange range){
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(_i2c, MPU6050_I2C_ADDR, GYRO_CONFIG, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&range, 1, HAL_MAX_DELAY);
    switch (range) {
        case GyroRange::DEG250:
            _gyroSensitivity = 131.0f;
            break;
        case GyroRange::DEG500:
            _gyroSensitivity = 65.5f;
            break;
        case GyroRange::DEG1000:
            _gyroSensitivity = 32.8f;
            break;
        case GyroRange::DEG2000:
            _gyroSensitivity = 16.4f;
            break;
    }
    
    return status;
}
HAL_StatusTypeDef MPU6050::setSampleRate(uint8_t divider){
    return HAL_I2C_Mem_Write(_i2c, MPU6050_I2C_ADDR, SMPLRT_DIV, I2C_MEMADD_SIZE_8BIT, &divider, 1, HAL_MAX_DELAY);
}

HAL_StatusTypeDef MPU6050::calibrateGyro(){
    float sumX = 0, sumY = 0, sumZ = 0;
    int numSamples = 500;
    for(int i = 0; i < numSamples; i++){
        getGyro();
        sumX += mpu6050_data.gyro_x;
        sumY += mpu6050_data.gyro_y;
        sumZ += mpu6050_data.gyro_z;
        HAL_Delay(1); // delay between samples
    }
    _gyroBiasX = sumX / numSamples;
    _gyroBiasY = sumY / numSamples;
    _gyroBiasZ = sumZ / numSamples;
    return HAL_OK;
}
