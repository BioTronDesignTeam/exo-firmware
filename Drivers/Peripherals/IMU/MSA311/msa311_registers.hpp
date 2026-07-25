#pragma once


static constexpr uint8_t MSA311_I2C_ADDR = 0x62 << 1;

static constexpr uint8_t MSA311_PARTID = 0x01;
static constexpr uint8_t MSA311_PARTID_VAL = 0x13;

static constexpr uint8_t MSA311_SOFT_RESET = 0x00;
static constexpr uint8_t MSA311_RANGE = 0x0F;
static constexpr uint8_t MSA311_ODR = 0x10;
static constexpr uint8_t MSA311_POWER_MODE = 0x11;
static constexpr uint8_t MSA311_NEW_DATA_INT = 0x0A;

static constexpr uint8_t MSA311_ACC_X_LSB = 0x02;
static constexpr uint8_t MSA311_ACC_X_MSB = 0x03;
static constexpr uint8_t MSA311_ACC_Y_LSB = 0x04;
static constexpr uint8_t MSA311_ACC_Y_MSB = 0x05;
static constexpr uint8_t MSA311_ACC_Z_LSB = 0x06;
static constexpr uint8_t MSA311_ACC_Z_MSB = 0x07;

enum class MSA311Range : uint8_t {
    RANGE_2G = 0x00,
    RANGE_4G = 0x01,
    RANGE_8G = 0x02,
    RANGE_16G = 0x03
};
enum class MSA311ODR : uint8_t {
    HZ_3_9 = 0x02,
    HZ_7_8 = 0x03,
    HZ_15_6 = 0x04,
    HZ_31_2 = 0x05,
    HZ_62_5 = 0x06,
    HZ_125 = 0x07,
    HZ_250 = 0x08
};
enum class MSA311PowerMode : uint8_t {
    NORMAL = 0x00 << 6,
    LOW_POWER = 0x01 << 6,
    SUSPEND = 0x02 << 6
};

