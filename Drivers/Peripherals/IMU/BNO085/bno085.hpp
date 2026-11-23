#pragma once

// Driver for the CEVA BNO085 9-axis IMU over I2C.
//
// Typical use:
//
//   BNO085 imu(&hi2c1, GPIOF, GPIO_PIN_3, false);
//   while (!imu.begin()) { /* wait and retry */ }
//
//   // In the task woken by the INT pin's falling edge:
//   imu.receiveReports();
//
//   // From any task:
//   bno085::Sample sample = imu.latestSample();
//
// Which reports the sensor streams, and how often, is set by kStreamedReports.

#include "stm32h7xx_hal.h"
#include "bno085_registers.hpp"

class BNO085 {
public:
    struct StreamedReport {
        uint8_t reportId;
        uint32_t intervalUs;
    };

    // Reports started by begin() and restarted after a sensor reset.
    // Their data ends up in bno085::Sample.
    static constexpr StreamedReport kStreamedReports[] = {
        {bno085::report::Accelerometer,           10000U},  // 100 Hz
        {bno085::report::GyroscopeCalibrated,     10000U},  // 100 Hz
        {bno085::report::MagneticFieldCalibrated, 10000U},  // 100 Hz
        {bno085::report::RotationVector,          10000U},  // 100 Hz
    };

    struct Diagnostics {
        uint32_t packetsReceived;
        uint32_t i2cErrors;
        uint32_t malformedPackets;
        uint16_t lastPacketLength;   // as announced by the packet header
        uint8_t lastPacketChannel;
        uint8_t lastPayloadPrefix[4];
    };

    // altAddress selects 0x4B (SA0 high) instead of 0x4A.
    // interruptPort may be nullptr; the driver then polls instead of
    // waiting for INT.
    BNO085(I2C_HandleTypeDef* i2c, GPIO_TypeDef* interruptPort, uint16_t interruptPin, bool altAddress);

    // Check that the sensor responds, then start kStreamedReports. Blocks for
    // up to ~350 ms. Returns false on failure; safe to call again to retry.
    bool begin();
    bool isInitialized() const;

    // Read every packet the sensor has waiting (bounded) and update the latest
    // sample. Call from the task woken by INT. Returns true if any packet was
    // handled.
    bool receiveReports();

    // A consistent copy of the newest readings. Safe to call from any task.
    bno085::Sample latestSample() const;

    const Diagnostics& diagnostics() const;

private:
    static constexpr uint16_t kHeaderLength = 4;
    static constexpr uint16_t kMaxPacketLength = 64;

    struct PacketHeader {
        uint16_t length;      // whole packet including header; may exceed kMaxPacketLength
        uint8_t channel;
        bool isContinuation;  // tail of a message too big for one packet
    };

    // Hardware
    I2C_HandleTypeDef* _i2c;
    GPIO_TypeDef* _interruptPort;
    uint16_t _interruptPin;
    uint16_t _i2cAddress;

    // SHTP transport
    uint8_t _rxPacket[kMaxPacketLength] = {};
    uint8_t _txPacket[kMaxPacketLength] = {};
    uint8_t _txSequence[bno085::channel::Count] = {};
    PacketHeader _rxHeader = {};

    // Sensor state
    bool _productIdReceived = false;
    bool _initialized = false;
    bno085::Sample _sample = {};
    Diagnostics _diagnostics = {};

    // Transport
    bool sendPacket(uint8_t channelId, const uint8_t* payload, uint16_t payloadLength);
    bool readPacket();
    bool isInterruptAsserted() const;

    // Incoming messages
    bool handlePacket();
    void handleInputReports(uint32_t receivedMs);
    void storeReport(const uint8_t* reportBytes, uint32_t receivedMs);

    // Commands
    bool enableReport(uint8_t reportId, uint32_t intervalUs);
    bool enableStreamedReports();

public:
    // -----------------------------------------------------------------------
    // Old field names, kept read-only so Tasks/Src/imu.cpp and uart.cpp still
    // build. They are NOT safe to read from another task (a value can change
    // halfway through a read). Delete this block once those files use
    // latestSample() and diagnostics().
    // Note: acceleration is now in m/s^2, not g.
    // -----------------------------------------------------------------------
    const bno085::Vector3Reading& acceleration = _sample.acceleration;
    const bno085::OrientationReading& rotationVector = _sample.orientation;
    const uint32_t& packetsReceived = _diagnostics.packetsReceived;
    const uint32_t& i2cErrors = _diagnostics.i2cErrors;
    const uint32_t& malformedPackets = _diagnostics.malformedPackets;
    const uint16_t& lastPacketLength = _diagnostics.lastPacketLength;
    const uint8_t& lastPacketChannel = _diagnostics.lastPacketChannel;
    const uint8_t (&lastPayloadPrefix)[4] = _diagnostics.lastPayloadPrefix;
};
