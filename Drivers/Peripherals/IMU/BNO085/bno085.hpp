#pragma once

#include "stm32h7xx_hal.h"
#include "bno085_registers.hpp"

class BNO085 {
private:
    static constexpr uint16_t SHTP_HEADER_LENGTH = 4;
    static constexpr uint16_t MAX_PACKET_LENGTH = 64;
    static constexpr uint8_t CHANNEL_COUNT = 6;

    I2C_HandleTypeDef* _hi2c;
    uint16_t _i2cAddress = BNO085_I2C_ADDR;
    uint8_t _rxBuffer[MAX_PACKET_LENGTH] = {0};
    uint8_t _txBuffer[MAX_PACKET_LENGTH] = {0};
    uint8_t _txSequence[CHANNEL_COUNT] = {0};
    uint16_t _rxLength = 0;
    uint8_t _rxChannel = 0;
    bool _rxContinuation = false;
    bool _discardingContinuation = false;
    bool _productIdReceived = false;
    bool _initialized = false;

    bool transmitPacket(uint8_t channel, const uint8_t* payload, uint16_t payloadLength);
    bool receivePacket(uint32_t timeoutMs);
    bool dispatchPacket();
    bool enableReport(uint8_t reportID, uint32_t intervalUs);
    void parseRotVector(uint16_t reportOffset);

public:
    bno085_accel_t acceleration = {0};
    bno085_rot_vector_t rotationVector = {0};
    uint32_t packetsReceived = 0;
    uint32_t i2cErrors = 0;
    uint32_t malformedPackets = 0;
    uint16_t lastPacketLength = 0;
    uint8_t lastPacketChannel = 0;
    uint8_t lastPayloadPrefix[4] = {0};

    explicit BNO085(I2C_HandleTypeDef* i2cHandle);
    bool begin();
    bool receiveReports();
    bool isInitialized() const;
};
