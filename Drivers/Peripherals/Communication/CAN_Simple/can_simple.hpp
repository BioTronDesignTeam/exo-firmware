#pragma once
/*
 * CAN Simple header file.
 *
 * @note: This is an internal representation of the CAN Simple Protocol used by Odrives
 * Link: https://docs.odriverobotics.com/v/latest/manual/can-protocol.html
 *
 * Author: Adityya Kaushal
 * Date of Creation: 2026-02-05
 */

// Command ID for each function
static constexpr uint8_t CMD_ID_GET_VERSION = 0x00;
static constexpr uint8_t CMD_ID_GET_HEARTBEAT = 0x01; //cyclic
static constexpr uint8_t CMD_ID_ESTOP = 0x02;
static constexpr uint8_t CMD_ID_GET_ERROR = 0x03; // cyclic
static constexpr uint8_t CMD_ID_MODIFY_PARAMETERS = 0x04; // RxSdo on documentation
static constexpr uint8_t CMD_ID_MODIFY_PARAMETERS_RESPONSE = 0x05 ;// TxSdo on documentation
static constexpr uint8_t CMD_ID_GET_ADDRESS = 0x06;
static constexpr uint8_t CMD_ID_SET_AXIS_STATE = 0x07;
static constexpr uint8_t CMD_ID_GET_ENCODE_ESTIMATES = 0x09; // cyclic
static constexpr uint8_t CMD_ID_SET_CONTROLLER_MODE = 0x0b;
static constexpr uint8_t CMD_ID_SET_INPUT_POSITION = 0x0c;
static constexpr uint8_t CMD_ID_SET_INPUT_VELOCITY = 0x0d;
static constexpr uint8_t CMD_ID_SET_INPUT_TORQUE = 0x0e;
static constexpr uint8_t CMD_ID_SET_LIMITS = 0x0f;
static constexpr uint8_t CMD_ID_SET_TRAJECTORY_VELOCITY_LIMIT = 0x11;
static constexpr uint8_t CMD_ID_SET_TRAJECTORY_ACCELERATION_LIMIT = 0x12;
static constexpr uint8_t CMD_ID_SET_TRAJECTORY_INERTIA = 0x13;
static constexpr uint8_t CMD_ID_GET_IQ = 0x14;
static constexpr uint8_t CMD_ID_GET_TEMPERATURE = 0x15;
static constexpr uint8_t CMD_ID_REBOOT = 0x16;
static constexpr uint8_t CMD_ID_GET_BUS_VOLTAGE_CURRENT = 0x17; // cyclic
static constexpr uint8_t CMD_ID_CLEAR_ERRORS = 0x18;
static constexpr uint8_t CMD_ID_SET_ABSOLUTE_POSITION = 0x19;
static constexpr uint8_t CMD_ID_SET_POSITION_GAIN = 0x1a;
static constexpr uint8_t CMD_ID_SET_VELOCITY_GAINS = 0x1b;
static constexpr uint8_t CMD_ID_GET_TORQUES = 0x1c; // cyclic
static constexpr uint8_t CMD_ID_GET_POWERS = 0x1d;
static constexpr uint8_t CMD_ID_ENTER_DFU_MODE = 0x1f;

// Possible Reset Modes
enum class ResetMode : uint8_t { // Explicitly set underlying type to char
    Reboot,
	SaveConfiguraton,
	EraseConfiguration,
	EnterDFUMode2
};

// Possible opCodes
enum class OpCode : uint8_t { // Explicitly set underlying type to char
    Read,
	Write
};

// Getters return structs
typedef struct {
	uint8_t protocolVersion; // This should always be 2 according to docs
	uint8_t hwVersionMajor;
	uint8_t hwVersionMinor;
	uint8_t hwVersionVariant;
	uint8_t fwVersionMajor;
	uint8_t fwVersionMinor;
	uint8_t fwVersionRevision;
	uint8_t fwVersionUnreleased;
} odrive_can_version_t;

typedef struct {
	uint32_t axisError;
	uint8_t axisState;
	uint8_t procedureResult;
	uint8_t trajectoryDoneFlag; // 0 = False, 1 = True
} odrive_can_heartbeat_t;

typedef struct {
	uint32_t activeErrors;
	uint32_t disarmReason;
} odrive_can_error_t;

// This message has an additional byte you have to set to 0!
typedef struct {
	uint8_t opCode; // 0 = read and 1 = write
	uint16_t endpointId;
	uint32_t value; // Depends on endpoint
} odrive_can_rxSdo_t;

// This message has two additional bytes you have to set to 0!
typedef struct {
	uint16_t endpointId;
	uint32_t value; // Depends on endpoint
} odrive_can_txSdo_t;

typedef struct {
	uint8_t nodeID;
	uint64_t serialNumber; // uint48_t
	uint8_t connectionID;
} odrive_can_address_t;

typedef struct {
	float positionEstimate;
	float velocityEstimate;
} odrive_can_encoder_estimates_t;

typedef struct {
	float iqSetpoint;
	float iqMeasured;
} odrive_can_iq_t;

typedef struct {
	float FETTemperature;
	float motorTemperature;
} odrive_can_temperature_t;

typedef struct {
	float busVoltage;
	float busCurrent;
} odrive_can_bus_t;

typedef struct {
	float torqueTarget;
	float torqueEstimate;
} odrive_can_torque_t;


typedef struct {
	float electricalPower;
	float mechanicalPower;
} odrive_can_power_t;
