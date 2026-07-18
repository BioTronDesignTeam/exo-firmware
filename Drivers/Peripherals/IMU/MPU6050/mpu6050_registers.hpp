#pragma once

// I2C address (AD0 pin low = 0x68, high = 0x69)
// Left-shifted by 1 as required by STM32 HAL
static constexpr uint8_t MPU6050_I2C_ADDR    = 0x68 << 1;
static constexpr uint8_t MPU6050_WHO_AM_I_VAL = 0x68; // expected WHO_AM_I response

// Self-test registers
static constexpr uint8_t SELF_TEST_X = 0x0D;
static constexpr uint8_t SELF_TEST_Y = 0x0E;
static constexpr uint8_t SELF_TEST_Z = 0x0F;
static constexpr uint8_t SELF_TEST_A = 0x10;

// Configuration registers
static constexpr uint8_t SMPLRT_DIV  = 0x19;
static constexpr uint8_t CONFIG      = 0x1A;
static constexpr uint8_t GYRO_CONFIG  = 0x1B;
static constexpr uint8_t ACCEL_CONFIG = 0x1C;

// Power management
static constexpr uint8_t PWR_MGMT_1 = 0x6B;
static constexpr uint8_t PWR_MGMT_2 = 0x6C;

// Sensor data registers — contiguous 0x3B–0x48 (14 bytes: accel + temp + gyro)
static constexpr uint8_t ACCEL_XOUT_H = 0x3B;
static constexpr uint8_t ACCEL_XOUT_L = 0x3C;
static constexpr uint8_t ACCEL_YOUT_H = 0x3D;
static constexpr uint8_t ACCEL_YOUT_L = 0x3E;
static constexpr uint8_t ACCEL_ZOUT_H = 0x3F;
static constexpr uint8_t ACCEL_ZOUT_L = 0x40;

static constexpr uint8_t TEMP_OUT_H = 0x41;
static constexpr uint8_t TEMP_OUT_L = 0x42;

static constexpr uint8_t GYRO_XOUT_H = 0x43;
static constexpr uint8_t GYRO_XOUT_L = 0x44;
static constexpr uint8_t GYRO_YOUT_H = 0x45;
static constexpr uint8_t GYRO_YOUT_L = 0x46;
static constexpr uint8_t GYRO_ZOUT_H = 0x47;
static constexpr uint8_t GYRO_ZOUT_L = 0x48;

static constexpr uint8_t USER_CTRL = 0x6A;
static constexpr uint8_t WHO_AM_I  = 0x75;

// Accel full scale range — written to bits [4:3] of ACCEL_CONFIG
enum class AccelRange : uint8_t {
    G2  = 0x00, // ±2g,  sensitivity: 16384 LSB/g
    G4  = 0x08, // ±4g,  sensitivity: 8192  LSB/g
    G8  = 0x10, // ±8g,  sensitivity: 4096  LSB/g
    G16 = 0x18  // ±16g, sensitivity: 2048  LSB/g
};

// Gyro full scale range — written to bits [4:3] of GYRO_CONFIG
enum class GyroRange : uint8_t {
    DEG250  = 0x00, // ±250  deg/s, sensitivity: 131.0 LSB/deg/s
    DEG500  = 0x08, // ±500  deg/s, sensitivity: 65.5  LSB/deg/s
    DEG1000 = 0x10, // ±1000 deg/s, sensitivity: 32.8  LSB/deg/s
    DEG2000 = 0x18  // ±2000 deg/s, sensitivity: 16.4  LSB/deg/s
};

// Data structs
typedef struct { float x, y, z; } mpu6050_accel_t; // units: g
typedef struct { float x, y, z; } mpu6050_gyro_t;  // units: deg/s
//typedef struct { float celsius;  } mpu6050_temp_t;
