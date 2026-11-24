#pragma once

// SH-2 / SHTP protocol constants and data types for the CEVA BNO085.
// IDs and formats are from the SH-2 Reference Manual (1000-3625) and the
// SHTP spec (1000-3535).

#include <cstdint>

namespace bno085 {

// ---------------------------------------------------------------------------
// I2C addresses, already shifted left for the STM32 HAL.
// ---------------------------------------------------------------------------
constexpr uint16_t kI2cAddress    = 0x4A << 1;  // SA0 pin low
constexpr uint16_t kI2cAddressAlt = 0x4B << 1;  // SA0 pin high

// ---------------------------------------------------------------------------
// SHTP channels.
// ---------------------------------------------------------------------------
namespace channel {
constexpr uint8_t Command            = 0;  // SHTP advertisement
constexpr uint8_t Executable         = 1;  // reset / sleep, reset-complete notice
constexpr uint8_t Control            = 2;  // SH-2 commands and responses
constexpr uint8_t InputReports       = 3;  // sensor data
constexpr uint8_t WakeInputReports   = 4;
constexpr uint8_t GyroRotationVector = 5;
constexpr uint8_t Count              = 6;
}  // namespace channel

// ---------------------------------------------------------------------------
// Messages on the control and executable channels.
// ---------------------------------------------------------------------------
namespace control {
constexpr uint8_t ProductIdResponse  = 0xF8;
constexpr uint8_t ProductIdRequest   = 0xF9;
constexpr uint8_t GetFeatureResponse = 0xFC;
constexpr uint8_t SetFeature         = 0xFD;
constexpr uint8_t SetFeatureLength   = 17;
}  // namespace control

namespace executable {
constexpr uint8_t ResetComplete = 0x01;
}  // namespace executable

// ---------------------------------------------------------------------------
// Input report IDs (channel 3).
// ---------------------------------------------------------------------------
namespace report {
constexpr uint8_t Accelerometer                    = 0x01;
constexpr uint8_t GyroscopeCalibrated              = 0x02;
constexpr uint8_t MagneticFieldCalibrated          = 0x03;
constexpr uint8_t LinearAcceleration               = 0x04;
constexpr uint8_t RotationVector                   = 0x05;
constexpr uint8_t Gravity                          = 0x06;
constexpr uint8_t GyroscopeUncalibrated            = 0x07;
constexpr uint8_t GameRotationVector               = 0x08;
constexpr uint8_t GeomagneticRotationVector        = 0x09;
constexpr uint8_t Pressure                         = 0x0A;
constexpr uint8_t AmbientLight                     = 0x0B;
constexpr uint8_t Humidity                         = 0x0C;
constexpr uint8_t Proximity                        = 0x0D;
constexpr uint8_t Temperature                      = 0x0E;
constexpr uint8_t MagneticFieldUncalibrated        = 0x0F;
constexpr uint8_t TapDetector                      = 0x10;
constexpr uint8_t StepCounter                      = 0x11;
constexpr uint8_t SignificantMotion                = 0x12;
constexpr uint8_t StabilityClassifier              = 0x13;
constexpr uint8_t RawAccelerometer                 = 0x14;
constexpr uint8_t RawGyroscope                     = 0x15;
constexpr uint8_t RawMagnetometer                  = 0x16;
constexpr uint8_t StepDetector                     = 0x18;
constexpr uint8_t ShakeDetector                    = 0x19;
constexpr uint8_t FlipDetector                     = 0x1A;
constexpr uint8_t PickupDetector                   = 0x1B;
constexpr uint8_t StabilityDetector                = 0x1C;
constexpr uint8_t PersonalActivityClassifier       = 0x1E;
constexpr uint8_t SleepDetector                    = 0x1F;
constexpr uint8_t TiltDetector                     = 0x20;
constexpr uint8_t PocketDetector                   = 0x21;
constexpr uint8_t CircleDetector                   = 0x22;
constexpr uint8_t HeartRateMonitor                 = 0x23;
constexpr uint8_t ArvrStabilizedRotationVector     = 0x28;
constexpr uint8_t ArvrStabilizedGameRotationVector = 0x29;
constexpr uint8_t GyroIntegratedRotationVector     = 0x2A;

// Not sensor data: timing records that precede sensor reports in a packet.
constexpr uint8_t TimestampRebase = 0xFA;
constexpr uint8_t BaseTimestamp   = 0xFB;
}  // namespace report

// Length in bytes of each input report, including its 4-byte report header
// (ID, sequence number, status, delay). The sensor packs several reports into
// one packet back to back, so this is how a parser finds the next one.
// Returns 0 for an ID whose length isn't known.
constexpr uint8_t inputReportLength(uint8_t reportId) {
    switch (reportId) {
    case report::TimestampRebase:
    case report::BaseTimestamp:
    case report::TapDetector:
        return 5;
    case report::Humidity:
    case report::Proximity:
    case report::Temperature:
    case report::SignificantMotion:
    case report::StabilityClassifier:
    case report::ShakeDetector:
    case report::FlipDetector:
    case report::PickupDetector:
    case report::StabilityDetector:
    case report::SleepDetector:
    case report::TiltDetector:
    case report::PocketDetector:
    case report::CircleDetector:
    case report::HeartRateMonitor:
        return 6;
    case report::Pressure:
    case report::AmbientLight:
    case report::StepDetector:
        return 8;
    case report::Accelerometer:
    case report::GyroscopeCalibrated:
    case report::MagneticFieldCalibrated:
    case report::LinearAcceleration:
    case report::Gravity:
        return 10;
    case report::GameRotationVector:
    case report::StepCounter:
    case report::ArvrStabilizedGameRotationVector:
        return 12;
    case report::RotationVector:
    case report::GeomagneticRotationVector:
    case report::ArvrStabilizedRotationVector:
    case report::GyroIntegratedRotationVector:
        return 14;
    case report::GyroscopeUncalibrated:
    case report::MagneticFieldUncalibrated:
    case report::RawAccelerometer:
    case report::RawGyroscope:
    case report::RawMagnetometer:
    case report::PersonalActivityClassifier:
        return 16;
    default:
        return 0;
    }
}

// Fixed-point formats: value = raw / 2^Q.
namespace qpoint {
constexpr int Accelerometer    = 8;   // m/s^2
constexpr int Gyroscope        = 9;   // rad/s
constexpr int MagneticField    = 4;   // uT
constexpr int Quaternion       = 14;  // unitless
constexpr int RotationAccuracy = 12;  // radians
}  // namespace qpoint

// ---------------------------------------------------------------------------
// Decoded data.
//
// status: the sensor's accuracy estimate for this reading,
//         0 = unreliable, 1 = low, 2 = medium, 3 = high.
// timeMs: HAL_GetTick() when the packet carrying the reading was read.
//         0 means no reading has arrived yet.
// ---------------------------------------------------------------------------
struct Vector3Reading {
    float x, y, z;
    uint8_t status;
    uint32_t timeMs;
};

struct OrientationReading {
    float i, j, k, real;    // unit quaternion
    float accuracyRadians;  // estimated heading error
    uint8_t status;
    uint32_t timeMs;
};

struct Sample {
    Vector3Reading acceleration;     // m/s^2, includes gravity
    Vector3Reading angularVelocity;  // rad/s, bias-corrected
    Vector3Reading magneticField;    // uT, calibrated
    OrientationReading orientation;  // fused from all three sensors
};

}  // namespace bno085

// Old type name still used by Tasks/Src/imu.cpp. Remove once it switches to
// bno085::OrientationReading.
using bno085_rot_vector_t = bno085::OrientationReading;
