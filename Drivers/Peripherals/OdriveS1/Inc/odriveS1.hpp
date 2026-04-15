#pragma once
/*
 * CAN driver for the Odrive S1
 *
 * Author: Adityya Kaushal
 * Date of Creation: 2026-02-01
 */

#ifndef INC_ODRIVES1_CAN_HPP_
#define INC_ODRIVES1_CAN_HPP_

#include "stm32H7xx_hal.h"
#include "stm32H7xx_hal_fdcan.h"
#include "CAN_simple.hpp"

class ODRIVES1 {
private:
	FDCAN_HandleTypeDef* _can;
	FDCAN_FilterTypeDef odriveCanFilter;

	// TODO: Replace test bytes
	uint8_t odriveTxBuffer[FDCAN_DLC_BYTES_8] =  {0};

public:
	// Internal States
	uint8_t odriveRxBuffer[FDCAN_DLC_BYTES_8] = {0};
	FDCAN_RxHeaderTypeDef odriveCanRxHeader= {0};
	odrive_can_version_t version = {0};
	odrive_can_heartbeat_t heartbeat = {0};
	odrive_can_error_t error = {0};
	odrive_can_address_t address = {0};
	odrive_can_encoder_estimates_t encoderEstimates = {0};
	odrive_can_iq_t iq = {0};
	odrive_can_temperature_t temperature = {0};
	odrive_can_bus_t busVoltageCurrent = {0};
	odrive_can_torque_t torque = {0};
	odrive_can_power_t power = {0};
	odrive_can_txSdo_t latestEndpointChange = {0};


	ODRIVES1 (FDCAN_HandleTypeDef* fdcanhandle);

	// CAN send function
	HAL_StatusTypeDef sendMsgCAN(uint32_t identifier, bool isRemote);

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
