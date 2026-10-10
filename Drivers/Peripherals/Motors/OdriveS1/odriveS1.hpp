#pragma once
/*
 * CAN driver for the Odrive S1
 *
 * Author: Adityya Kaushal
 * Date of Creation: 2026-02-01
 */

#ifndef INC_ODRIVES1_CAN_HPP_
#define INC_ODRIVES1_CAN_HPP_

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_fdcan.h"
#include "can_simple.hpp"

#ifndef ODRIVE_CAN_NODE_ID
#define ODRIVE_CAN_NODE_ID 0
#endif
static_assert(ODRIVE_CAN_NODE_ID >= 0 && ODRIVE_CAN_NODE_ID < 63,
              "ODrive node ID must be 0-62; 63 is the broadcast address");

class ODRIVES1 {
private:
	FDCAN_HandleTypeDef* _can;
	FDCAN_FilterTypeDef odriveCanFilter{};
    bool _initialized = false;
    volatile bool _heartbeatReceived = false;
    volatile uint32_t _heartbeatTick = 0;

public:
	// Internal States
	uint8_t odriveRxBuffer[FDCAN_DLC_BYTES_8] = {};
	FDCAN_RxHeaderTypeDef odriveCanRxHeader= {};
	odrive_can_version_t version = {};
	odrive_can_heartbeat_t heartbeat = {};
	odrive_can_error_t error = {};
	odrive_can_address_t address = {};
	odrive_can_encoder_estimates_t encoderEstimates = {};
	odrive_can_iq_t iq = {};
	odrive_can_temperature_t temperature = {};
	odrive_can_bus_t busVoltageCurrent = {};
	odrive_can_torque_t torque = {};
	odrive_can_power_t power = {};
	odrive_can_txSdo_t latestEndpointChange = {};


	ODRIVES1 (FDCAN_HandleTypeDef* fdcanhandle);

	bool isInitialized() const { return _initialized; }
    // Copy ISR-updated heartbeat and timestamp together.
    bool heartbeatSnapshot(odrive_can_heartbeat_t& status, uint32_t& tick) const;

	// CAN send function
	HAL_StatusTypeDef sendMsgCAN(uint32_t identifier, bool isRemote, const uint8_t* txBuffer = nullptr);

	// Getters
	HAL_StatusTypeDef getVersion();
	HAL_StatusTypeDef getHeartbeat();
	HAL_StatusTypeDef getError();
	HAL_StatusTypeDef getCANAddress();
	HAL_StatusTypeDef getEncoderEstimates();
	HAL_StatusTypeDef getIq();
	HAL_StatusTypeDef getTemperatures();
	HAL_StatusTypeDef getBusVoltageCurrent();
	HAL_StatusTypeDef getTorques();
	HAL_StatusTypeDef getPowers();

	// Callback for messages from odrive
	HAL_StatusTypeDef responseCallback(uint32_t identifier);

	// TODO: Add proper parameters to following sections
	// Setters
	HAL_StatusTypeDef setAxisState(uint32_t requestedState);
	HAL_StatusTypeDef setControllerMode(uint32_t controlMode, uint32_t inputMode);
	HAL_StatusTypeDef setInputPosition(float inputPos, int16_t inputVel, int16_t inputTorque);
	HAL_StatusTypeDef setInputVelocity(float inputVel, float inputTorque);
	HAL_StatusTypeDef setInputTorque(float inputTorque);
	HAL_StatusTypeDef setLimits(float velLimit, float currentSoftMax);
	HAL_StatusTypeDef setTrajectoryVelocityLimit(float velocityLimit);
	HAL_StatusTypeDef setTrajectoryAccelerationLimit(float accelerationLimit, float decelerationLimit);
	HAL_StatusTypeDef setTrajectoryInertia(float inertia);
	HAL_StatusTypeDef setAbsolutePosition(float postionEstimate);
	HAL_StatusTypeDef setPositionGain(float postionGain);
	HAL_StatusTypeDef setVelocityGain(float velocityGain, float velocityIntegratorGain);

	// Functions that alter functionality
	// RxSdo
	HAL_StatusTypeDef modifyParameter(OpCode opCode, uint16_t endpointID, uint32_t value);
	HAL_StatusTypeDef rebootOdrive(ResetMode resetMode);
	HAL_StatusTypeDef clearErrors(uint8_t identify);

	// No data, empty frame
	HAL_StatusTypeDef enterDFUMode();
};

#endif /* INC_ODRIVES1_CAN_HPP_ */
