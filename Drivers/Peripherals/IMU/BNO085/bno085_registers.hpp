#pragma once

// I2C address (SA0 pin low = 0x4a << 1)
static constexpr uint8_t BNO085_I2C_ADDR = 0x4a << 1;
static constexpr uint8_t BNO085_I2C_ADDR_ALT = 0x4b << 1;

// channels
static constexpr uint8_t EXECUTABLE_CONTROL = 0x01;
static constexpr uint8_t SENSOR_HUB_CONTROL = 0x02; //enable reports
static constexpr uint8_t STANDARD_REPORTS = 0x03; //report requests

// sensor data
static constexpr uint8_t ACCEL = 0x01;
static constexpr uint8_t GYRO = 0x02;
static constexpr uint8_t CALIBRATE_MAG_FIELD = 0x03;
static constexpr uint8_t LIN_ACCEL = 0x04;
static constexpr uint8_t ROT_VECTOR = 0x05;
static constexpr uint8_t GRAVITY = 0x06;
static constexpr uint8_t CALIBRATE_GYRO = 0x07;
static constexpr uint8_t GAME_ROT_VECTOR = 0x08;
static constexpr uint8_t GEOMAG_ROT_VECTOR = 0x09;
static constexpr uint8_t PRESSURE = 0x0A;
static constexpr uint8_t AMB_LIGHT = 0x0B;
static constexpr uint8_t HUMIDITY = 0x0C;
static constexpr uint8_t PROXIMITY = 0x0D;
static constexpr uint8_t TEMPERATURE = 0x0E;
static constexpr uint8_t MAG_FIELD = 0x0F;
static constexpr uint8_t TAP_DETECTOR = 0x10;
static constexpr uint8_t STEP_COUNTER = 0x11;
static constexpr uint8_t SIG_MOTION = 0x12;
static constexpr uint8_t STABILITY_CLASSIFER = 0x13;
static constexpr uint8_t RAW_ACCEL = 0x14;
static constexpr uint8_t RAW_GYRO = 0x15;
static constexpr uint8_t RAW_MAG = 0x16;
static constexpr uint8_t SAR = 0x17;
static constexpr uint8_t STEP_DETECTOR = 0x18;
static constexpr uint8_t SHAKE_DETECTOR = 0x19;
static constexpr uint8_t FLIP_DETECTOR = 0x1A;
static constexpr uint8_t PICKUP_DETECTOR = 0x1B;
static constexpr uint8_t STABILITY_DETECTOR = 0x1C;
static constexpr uint8_t PERSONAL_ACTIVITY_CLASSIFER = 0x1E;
static constexpr uint8_t SLEEP_DETECTOR = 0x1F;
static constexpr uint8_t TILT_DETECTOR = 0x20;
static constexpr uint8_t POCKET_DETECTOR = 0x21;
static constexpr uint8_t CIRCLE_DETECTOR = 0x22;
static constexpr uint8_t HEART_RATE_MONITOR = 0x23;

static constexpr uint8_t SET_FEATURE_COMMAND = 0xFD;
static constexpr uint8_t PRODUCT_ID_REQUEST = 0xF9;
static constexpr uint8_t PRODUCT_ID_RESPONSE = 0xF8;
static constexpr uint8_t RESET_COMPLETE = 0x01;

typedef struct { float x, y, z; } bno085_accel_t; // units: g
typedef struct {
    float i, j, k, real;
    float accuracyRadians;
    uint8_t status;
} bno085_rot_vector_t;
typedef struct { float x, y, z; } bno085_gyro_t; // units: deg/s
