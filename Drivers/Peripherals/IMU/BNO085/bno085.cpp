// BNO085 driver. See bno085.hpp for how to use it.
//
// The code is layered bottom-up:
//
//   Transport   sendPacket() / readPacket() move one SHTP packet
//               (4-byte header + payload) over I2C.
//   Messages    handlePacket() routes a received packet by channel: control
//               responses, reset notices, sensor data.
//   Reports     handleInputReports() walks the sensor reports in a packet;
//               storeReport() decodes one into _sample.

#include "bno085.hpp"

#include <cstring>

#include "cmsis_os2.h"

using namespace bno085;

namespace {

constexpr uint32_t kBootDelayMs         = 100U;  // sensor firmware boot time after power-up
constexpr uint32_t kSensorReplyTimeoutMs = 250U; // how long begin() waits for an answer
constexpr uint32_t kI2cTimeoutMs        = 50U;
constexpr uint8_t  kMaxPacketsPerDrain  = 8U;    // upper bound on one receiveReports() call

// SHTP header: bytes 0-1 length (little-endian, includes the header),
// bit 15 of the length = continuation flag, byte 2 = channel,
// byte 3 = per-channel sequence number.
constexpr uint16_t kContinuationFlag = 0x8000U;
constexpr uint16_t kLengthMask       = 0x7FFFU;

// Sensor report header: [0] ID, [1] sequence, [2] status, [3] delay.
constexpr uint8_t kReportStatusByte   = 2U;
constexpr uint8_t kReportDataOffset   = 4U;
constexpr uint8_t kStatusAccuracyMask = 0x03U;

// ---------------------------------------------------------------------------
// Byte and fixed-point helpers. Everything on the wire is little-endian.
// ---------------------------------------------------------------------------
uint16_t readUint16Le(const uint8_t* bytes) {
    return static_cast<uint16_t>(static_cast<uint16_t>(bytes[0]) |
                                 (static_cast<uint16_t>(bytes[1]) << 8));
}

void writeUint32Le(uint8_t* bytes, uint32_t value) {
    bytes[0] = static_cast<uint8_t>(value);
    bytes[1] = static_cast<uint8_t>(value >> 8);
    bytes[2] = static_cast<uint8_t>(value >> 16);
    bytes[3] = static_cast<uint8_t>(value >> 24);
}

// Read a signed 16-bit fixed-point value with qBits fractional bits.
float readFixedPoint(const uint8_t* bytes, int qBits) {
    const int16_t raw = static_cast<int16_t>(readUint16Le(bytes));
    return static_cast<float>(raw) / static_cast<float>(1 << qBits);
}

// x, y, z at data offsets 0, 2, 4 of a 3-axis report.
Vector3Reading decodeVector3(const uint8_t* reportBytes, int qBits, uint32_t receivedMs) {
    const uint8_t* data = reportBytes + kReportDataOffset;
    Vector3Reading reading = {};
    reading.x = readFixedPoint(data + 0, qBits);
    reading.y = readFixedPoint(data + 2, qBits);
    reading.z = readFixedPoint(data + 4, qBits);
    reading.status = reportBytes[kReportStatusByte] & kStatusAccuracyMask;
    reading.timeMs = receivedMs;
    return reading;
}

// Rotation vector: i, j, k, real (Q14), then heading accuracy (Q12 radians).
OrientationReading decodeRotationVector(const uint8_t* reportBytes, uint32_t receivedMs) {
    const uint8_t* data = reportBytes + kReportDataOffset;
    OrientationReading reading = {};
    reading.i = readFixedPoint(data + 0, qpoint::Quaternion);
    reading.j = readFixedPoint(data + 2, qpoint::Quaternion);
    reading.k = readFixedPoint(data + 4, qpoint::Quaternion);
    reading.real = readFixedPoint(data + 6, qpoint::Quaternion);
    reading.accuracyRadians = readFixedPoint(data + 8, qpoint::RotationAccuracy);
    reading.status = reportBytes[kReportStatusByte] & kStatusAccuracyMask;
    reading.timeMs = receivedMs;
    return reading;
}

// ---------------------------------------------------------------------------
// RTOS helpers. Both fall back to plain behaviour before the scheduler starts.
// ---------------------------------------------------------------------------

// Wait without spinning the CPU once the RTOS is running. HAL_Delay busy-waits
// and would starve every lower-priority task while begin() waits.
void sleepMs(uint32_t milliseconds) {
    if (osKernelGetState() != osKernelRunning) {
        HAL_Delay(milliseconds);
        return;
    }
    const uint32_t ticks = (milliseconds * osKernelGetTickFreq() + 999U) / 1000U;
    osDelay(ticks == 0U ? 1U : ticks);
}

// Stops task switches for its lifetime, so _sample is never read while half
// written. Held only for a struct copy, so the pause is a few microseconds.
class SchedulerLock {
public:
    SchedulerLock()
        : _locked(osKernelGetState() == osKernelRunning),
          _previousState(_locked ? osKernelLock() : 0) {}
    ~SchedulerLock() {
        if (_locked) {
            osKernelRestoreLock(_previousState);
        }
    }
    SchedulerLock(const SchedulerLock&) = delete;
    SchedulerLock& operator=(const SchedulerLock&) = delete;

private:
    bool _locked;
    int32_t _previousState;
};

}  // namespace

// ===========================================================================
// Public interface
// ===========================================================================

BNO085::BNO085(I2C_HandleTypeDef* i2c, GPIO_TypeDef* interruptPort, uint16_t interruptPin, bool altAddress)
    : _i2c(i2c),
      _interruptPort(interruptPort),
      _interruptPin(interruptPin),
      _i2cAddress(altAddress ? kI2cAddressAlt : kI2cAddress) {}

// Ask for the product ID and wait for any sign of life. The sensor counts as
// alive if it sends:
//   - the product ID response (normal power-up),
//   - a reset-complete notice (handlePacket() has already restarted the reports),
//   - sensor data (only the MCU was reset; the sensor kept streaming).
bool BNO085::begin() {
    _initialized = false;
    _productIdReceived = false;
    if (_i2c == nullptr) {
        return false;
    }
    memset(_txSequence, 0, sizeof(_txSequence));

    sleepMs(kBootDelayMs);

    const uint8_t productIdRequest[] = {control::ProductIdRequest};
    if (!sendPacket(channel::Control, productIdRequest, sizeof(productIdRequest))) {
        return false;
    }

    const uint32_t startMs = HAL_GetTick();
    while ((HAL_GetTick() - startMs) < kSensorReplyTimeoutMs) {
        // The sensor only sends a real packet while INT is low; reading at
        // other times returns filler.
        if (!isInterruptAsserted() || !readPacket()) {
            sleepMs(1U);
            continue;
        }

        handlePacket();

        if (_initialized) {
            return true;
        }
        const bool sensorAlreadyStreaming = (_rxHeader.channel == channel::InputReports);
        if (_productIdReceived || sensorAlreadyStreaming) {
            _initialized = enableStreamedReports();
            return _initialized;
        }
    }
    return false;
}

bool BNO085::isInitialized() const {
    return _initialized;
}

// INT stays low while the sensor has more data queued, so reading only one
// packet per falling edge could leave it waiting forever for a read that
// never comes. Keep reading until INT goes high, up to a fixed limit.
bool BNO085::receiveReports() {
    bool handledAnyPacket = false;
    for (uint8_t count = 0; count < kMaxPacketsPerDrain; ++count) {
        if (!isInterruptAsserted() || !readPacket()) {
            break;
        }
        if (handlePacket()) {
            handledAnyPacket = true;
        }
    }
    return handledAnyPacket;
}

Sample BNO085::latestSample() const {
    SchedulerLock lock;
    return _sample;
}

const BNO085::Diagnostics& BNO085::diagnostics() const {
    return _diagnostics;
}

// ===========================================================================
// Incoming messages
// ===========================================================================

// Act on the packet in _rxPacket. Returns false if it was skipped.
bool BNO085::handlePacket() {
    // A message bigger than _rxPacket arrives as a first packet whose header
    // length exceeds the buffer, then packets flagged as continuations. They
    // aren't reassembled here, so both parts are skipped.
    const bool isTruncated = _rxHeader.length > sizeof(_rxPacket);
    const bool hasPayload = _rxHeader.length > kHeaderLength;
    if (_rxHeader.isContinuation || isTruncated || !hasPayload) {
        return false;
    }

    const uint8_t messageId = _rxPacket[kHeaderLength];

    switch (_rxHeader.channel) {
    case channel::Control:
        if (messageId == control::ProductIdResponse) {
            _productIdReceived = true;
        }
        return true;

    case channel::Executable:
        if (messageId == executable::ResetComplete) {
            // A sensor reset turns every report off; start them again.
            _initialized = enableStreamedReports();
        }
        return true;

    case channel::InputReports:
        handleInputReports(HAL_GetTick());
        return true;

    default:
        return true;  // valid packet on a channel this driver doesn't use
    }
}

// The payload is a run of back-to-back reports, often led by a timestamp.
// Walk them all; stop at an unknown ID, since the next report can't be found
// without knowing this one's length.
void BNO085::handleInputReports(uint32_t receivedMs) {
    uint16_t offset = kHeaderLength;
    while (offset < _rxHeader.length) {
        const uint8_t* reportBytes = _rxPacket + offset;
        const uint8_t reportLength = inputReportLength(reportBytes[0]);
        if (reportLength == 0U) {
            return;
        }
        if (offset + reportLength > _rxHeader.length) {
            ++_diagnostics.malformedPackets;  // report cut off by the end of the packet
            return;
        }
        storeReport(reportBytes, receivedMs);
        offset += reportLength;
    }
}

// Decode one complete report into _sample. Reports this driver doesn't keep
// (timestamps, anything not in Sample) are ignored.
void BNO085::storeReport(const uint8_t* reportBytes, uint32_t receivedMs) {
    switch (reportBytes[0]) {
    case report::Accelerometer: {
        const Vector3Reading reading = decodeVector3(reportBytes, qpoint::Accelerometer, receivedMs);
        SchedulerLock lock;
        _sample.acceleration = reading;
        break;
    }
    case report::GyroscopeCalibrated: {
        const Vector3Reading reading = decodeVector3(reportBytes, qpoint::Gyroscope, receivedMs);
        SchedulerLock lock;
        _sample.angularVelocity = reading;
        break;
    }
    case report::MagneticFieldCalibrated: {
        const Vector3Reading reading = decodeVector3(reportBytes, qpoint::MagneticField, receivedMs);
        SchedulerLock lock;
        _sample.magneticField = reading;
        break;
    }
    case report::RotationVector: {
        const OrientationReading reading = decodeRotationVector(reportBytes, receivedMs);
        SchedulerLock lock;
        _sample.orientation = reading;
        break;
    }
    default:
        break;
    }
}

// ===========================================================================
// Commands
// ===========================================================================

// Ask the sensor to send reportId every intervalUs microseconds.
// Set Feature command (17 bytes):
//   [0] 0xFD  [1] report ID  [2] flags  [3-4] change sensitivity
//   [5-8] report interval (us)  [9-12] batch interval  [13-16] sensor config
// Fields this driver doesn't use stay zero.
bool BNO085::enableReport(uint8_t reportId, uint32_t intervalUs) {
    uint8_t command[control::SetFeatureLength] = {};
    command[0] = control::SetFeature;
    command[1] = reportId;
    writeUint32Le(&command[5], intervalUs);
    return sendPacket(channel::Control, command, sizeof(command));
}

// Out-of-line definition: required when building as C++14 (the STM32CubeIDE
// default), where a static constexpr array member is only declared in the
// class. Redundant but harmless under C++17 (the CMake build).
constexpr BNO085::StreamedReport BNO085::kStreamedReports[];

// Enable every entry in kStreamedReports. Tries them all even if one fails,
// and returns true only if all succeeded.
bool BNO085::enableStreamedReports() {
    bool allEnabled = true;
    for (const StreamedReport& streamed : kStreamedReports) {
        if (!enableReport(streamed.reportId, streamed.intervalUs)) {
            allEnabled = false;
        }
    }
    return allEnabled;
}

// ===========================================================================
// SHTP transport
// ===========================================================================

// INT is active-low: the sensor pulls it low while it has a packet to send.
// Without an INT pin, assume data may be waiting and let the read decide.
bool BNO085::isInterruptAsserted() const {
    if (_interruptPort == nullptr) {
        return true;
    }
    return HAL_GPIO_ReadPin(_interruptPort, _interruptPin) == GPIO_PIN_RESET;
}

// Send one packet: 4-byte header followed by payload.
bool BNO085::sendPacket(uint8_t channelId, const uint8_t* payload, uint16_t payloadLength) {
    const uint16_t packetLength = payloadLength + kHeaderLength;
    if (channelId >= channel::Count || payload == nullptr || packetLength > sizeof(_txPacket)) {
        return false;
    }

    _txPacket[0] = static_cast<uint8_t>(packetLength);
    _txPacket[1] = static_cast<uint8_t>(packetLength >> 8);
    _txPacket[2] = channelId;
    _txPacket[3] = _txSequence[channelId]++;
    memcpy(_txPacket + kHeaderLength, payload, payloadLength);

    if (HAL_I2C_Master_Transmit(_i2c, _i2cAddress, _txPacket, packetLength, kI2cTimeoutMs) != HAL_OK) {
        ++_diagnostics.i2cErrors;
        return false;
    }
    return true;
}

// Read one packet into _rxPacket and decode its header into _rxHeader.
// Returns false if there was nothing to read or the header is invalid.
bool BNO085::readPacket() {
    // The sensor restarts its output on every I2C STOP, so a packet has to be
    // read in one transaction. Read a full buffer; the header says how much
    // of it is real.
    if (HAL_I2C_Master_Receive(_i2c, _i2cAddress, _rxPacket, sizeof(_rxPacket), kI2cTimeoutMs) != HAL_OK) {
        ++_diagnostics.i2cErrors;
        return false;
    }

    const uint16_t lengthField = readUint16Le(_rxPacket);
    _rxHeader.isContinuation = (lengthField & kContinuationFlag) != 0U;
    _rxHeader.length = lengthField & kLengthMask;
    _rxHeader.channel = _rxPacket[2];

    if (_rxHeader.length == 0U) {
        return false;  // sensor had nothing to send; not an error
    }

    _diagnostics.lastPacketLength = _rxHeader.length;
    _diagnostics.lastPacketChannel = _rxHeader.channel;

    if (_rxHeader.length < kHeaderLength || _rxHeader.channel >= channel::Count) {
        ++_diagnostics.malformedPackets;
        return false;
    }

    const uint16_t bytesInBuffer = (_rxHeader.length < sizeof(_rxPacket)) ? _rxHeader.length : sizeof(_rxPacket);
    const uint16_t payloadBytes = bytesInBuffer - kHeaderLength;
    const uint16_t prefixBytes = (payloadBytes < sizeof(_diagnostics.lastPayloadPrefix))
                                     ? payloadBytes
                                     : sizeof(_diagnostics.lastPayloadPrefix);
    memset(_diagnostics.lastPayloadPrefix, 0, sizeof(_diagnostics.lastPayloadPrefix));
    memcpy(_diagnostics.lastPayloadPrefix, _rxPacket + kHeaderLength, prefixBytes);

    ++_diagnostics.packetsReceived;
    return true;
}
