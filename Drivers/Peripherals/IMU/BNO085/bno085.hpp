#pragma once

#include "stm32H7xx_hal.h"
#include "BNO085_registers.hpp"

class BNO085 {
private:
    I2C_HandleTypeDef* _hi2c; //pointer to manager of this i2c port
    uint8_t _rxBuffer[64] = {0}; //receive buffer
    uint8_t _txBuffer[64] = {0}; //transmit buffer

    void transmitPacket(uint8_t channel, uint8_t* payload, uint8_t payloadLength);
    void enableReport(uint8_t reportID, uint32_t intervalUs);
    void parseRotVector();

public:
    bno085_accel_t acceleration = {0};
    bno085_rot_vector_t rotationVector = {0};
    BNO085(I2C_HandleTypeDef* i2cHandle); //constructor
    bool begin(); // verifies product ID and enables reports
    bool receiveReports(); // master receives and routes to parsing functions
};
