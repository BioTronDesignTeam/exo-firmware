#include "bno085.hpp"
#include <string.h>

BNO085::BNO085(I2C_HandleTypeDef* i2cHandle) : _hi2c(i2cHandle) {}

bool BNO085::begin() {
    if (_hi2c == nullptr) {
        return false;
    }

    _initialized = false;
    _productIdReceived = false;
    memset(_txSequence, 0, sizeof(_txSequence));

    // The BNO085 may advertise itself immediately after power-up. Give its
    // firmware time to boot, then request product information explicitly.
    HAL_Delay(100);
    const uint8_t productIdRequest[] = {PRODUCT_ID_REQUEST};
    const uint16_t addresses[] = {BNO085_I2C_ADDR, BNO085_I2C_ADDR_ALT};
    for (const uint16_t address : addresses) {
        _i2cAddress = address;
        _productIdReceived = false;
        memset(_txSequence, 0, sizeof(_txSequence));

        if (!transmitPacket(SENSOR_HUB_CONTROL, productIdRequest, sizeof(productIdRequest))) {
            continue;
        }

        const uint32_t deadline = HAL_GetTick() + 250U;
        while ((int32_t)(HAL_GetTick() - deadline) < 0) {
            // The BNO085 only presents a valid SHTP header while INT is
            // asserted. Reading while it is high returns filler bytes that
            // do not form a packet.
            if (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6) == GPIO_PIN_RESET &&
                receivePacket(50U)) {
                dispatchPacket();
                if (_productIdReceived) {
                    _initialized = enableReport(ROT_VECTOR, 10000U);
                    return _initialized;
                }
                // A MCU-only reset leaves the BNO085 running and it may
                // already be streaming reports. A valid input-report packet
                // proves the transport is alive, so restore the requested
                // feature instead of waiting indefinitely for product ID.
                if (_rxChannel == STANDARD_REPORTS) {
                    _initialized = enableReport(ROT_VECTOR, 10000U);
                    return _initialized;
                }
            } else {
                HAL_Delay(1U);
            }
        }
    }

    return false;
}

bool BNO085::enableReport(uint8_t reportID, uint32_t intervalUs) {
    uint8_t payload[17] = {0};
    payload[0] = SET_FEATURE_COMMAND;
    payload[1] = reportID;
    payload[5] = intervalUs & 0xFF;
    payload[6] = (intervalUs >> 8) & 0xFF;
    payload[7] = (intervalUs >> 16) & 0xFF;
    payload[8] = (intervalUs >> 24) & 0xFF;

    return transmitPacket(SENSOR_HUB_CONTROL, payload, sizeof(payload));
}

bool BNO085::transmitPacket(uint8_t channel, const uint8_t* payload, uint16_t payloadLength) {
    const uint16_t totalLength = payloadLength + SHTP_HEADER_LENGTH;
    if (channel >= CHANNEL_COUNT || payload == nullptr || totalLength > sizeof(_txBuffer)) {
        return false;
    }

    _txBuffer[0] = static_cast<uint8_t>(totalLength);
    _txBuffer[1] = static_cast<uint8_t>(totalLength >> 8);
    _txBuffer[2] = channel;
    _txBuffer[3] = _txSequence[channel]++;
    memcpy(_txBuffer + SHTP_HEADER_LENGTH, payload, payloadLength);

    if (HAL_I2C_Master_Transmit(_hi2c, _i2cAddress, _txBuffer, totalLength, 100U) != HAL_OK) {
        ++i2cErrors;
        return false;
    }
    return true;
}

bool BNO085::receivePacket(uint32_t timeoutMs) {
    // The BNO085 I2C transport restarts its read cursor on every STOP. Read
    // a complete transport frame in one transaction, then use its SHTP header
    // to determine how many received bytes are meaningful.
    if (HAL_I2C_Master_Receive(_hi2c, _i2cAddress, _rxBuffer, sizeof(_rxBuffer), timeoutMs) != HAL_OK) {
        ++i2cErrors;
        return false;
    }

    const uint16_t encodedLength = static_cast<uint16_t>(_rxBuffer[0]) |
                                   (static_cast<uint16_t>(_rxBuffer[1]) << 8);
    _rxContinuation = (encodedLength & 0x8000U) != 0U;
    _rxLength = encodedLength & 0x7FFFU;
    _rxChannel = _rxBuffer[2];
    lastPacketLength = _rxLength;
    lastPacketChannel = _rxChannel;

    if (_rxLength < SHTP_HEADER_LENGTH || _rxLength > sizeof(_rxBuffer)) {
        ++malformedPackets;
        return false;
    }

    const uint16_t payloadLength = _rxLength - SHTP_HEADER_LENGTH;
    const uint16_t prefixLength = (payloadLength < sizeof(lastPayloadPrefix))
                                      ? payloadLength
                                      : sizeof(lastPayloadPrefix);
    memset(lastPayloadPrefix, 0, sizeof(lastPayloadPrefix));
    memcpy(lastPayloadPrefix, _rxBuffer + SHTP_HEADER_LENGTH, prefixLength);
    ++packetsReceived;
    return true;
}

bool BNO085::receiveReports() {
    if (!receivePacket(50U)) {
        return false;
    }
    return dispatchPacket();
}

bool BNO085::dispatchPacket() {
    // This driver does not support reassembling fragmented SHTP messages.
    // Drop every fragment in the message rather than parsing its tail as a
    // new report.
    if (_discardingContinuation || _rxContinuation) {
        _discardingContinuation = _rxContinuation;
        return false;
    }

    const uint16_t payloadLength = _rxLength - SHTP_HEADER_LENGTH;
    if (payloadLength == 0U) {
        return false;
    }

    if (_rxChannel == SENSOR_HUB_CONTROL && _rxBuffer[4] == PRODUCT_ID_RESPONSE) {
        _productIdReceived = true;
        return true;
    }

    if (_rxChannel == EXECUTABLE_CONTROL && _rxBuffer[4] == RESET_COMPLETE) {
        // Sensor reset clears enabled features; restore the configured stream.
        _initialized = enableReport(ROT_VECTOR, 10000U);
        return true;
    }

    if (_rxChannel == STANDARD_REPORTS) {
        // Input reports may start with a base timestamp (0xFB). The actual
        // sensor report follows its five-byte timestamp payload.
        uint16_t reportOffset = SHTP_HEADER_LENGTH;
        if (_rxBuffer[reportOffset] == 0xFBU) {
            reportOffset += 5U;
        }
        if (reportOffset < _rxLength && _rxBuffer[reportOffset] == ROT_VECTOR) {
            parseRotVector(reportOffset);
        }
        return true;
    }

    return true;
}

void BNO085::parseRotVector(uint16_t reportOffset) {
    // Report ID, sequence, status, delay, then i/j/k/real in Q14 and
    // accuracy in Q12 radians.
    if (_rxLength < reportOffset + 14U) {
        ++malformedPackets;
        return;
    }

    const auto readLe16 = [this](uint16_t offset) {
        return static_cast<int16_t>(static_cast<uint16_t>(_rxBuffer[offset]) |
                                    (static_cast<uint16_t>(_rxBuffer[offset + 1]) << 8));
    };
    constexpr float quaternionScale = 1.0f / 16384.0f;
    constexpr float accuracyScale = 1.0f / 4096.0f;

    rotationVector.status = _rxBuffer[reportOffset + 2U] & 0x03U;
    rotationVector.i = readLe16(reportOffset + 4U) * quaternionScale;
    rotationVector.j = readLe16(reportOffset + 6U) * quaternionScale;
    rotationVector.k = readLe16(reportOffset + 8U) * quaternionScale;
    rotationVector.real = readLe16(reportOffset + 10U) * quaternionScale;
    rotationVector.accuracyRadians = readLe16(reportOffset + 12U) * accuracyScale;
}

bool BNO085::isInitialized() const {
    return _initialized;
}

