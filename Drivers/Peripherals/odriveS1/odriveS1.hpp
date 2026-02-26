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
	FDCAN_RxHeaderTypeDef odriveCanRxHeader;
	FDCAN_FilterTypeDef odriveCanFilter;

	// TODO: Replace test bytes
	uint8_t odriveTxBuffer[FDCAN_DLC_BYTES_8] =  {0};
	uint8_t odriveRxBuffer[FDCAN_DLC_BYTES_8] = {0};

public:
	ODRIVES1 (FDCAN_HandleTypeDef* fdcanhandle);

	// Can send function
	HAL_StatusTypeDef sendGetMsgCAN(uint32_t identifier);
	HAL_StatusTypeDef sendSetMsgCAN(uint32_t identifier);

	// Getters
	HAL_StatusTypeDef getVersion(odrive_can_version_t* version);
	HAL_StatusTypeDef getHeartbeat(odrive_can_heartbeat_t* heartbeat);
	HAL_StatusTypeDef getError(odrive_can_error_t* error);
	HAL_StatusTypeDef getCANAddress(odrive_can_address_t* address);
	HAL_StatusTypeDef getEncoderEstimates(odrive_can_encoder_estimates_t* encoderEstimates);
	HAL_StatusTypeDef getIq(odrive_can_iq_t* iq);
	HAL_StatusTypeDef getTemperatures(odrive_can_temperature_t* temperature);
	HAL_StatusTypeDef getBusVoltageCurrent(odrive_can_bus_t* busVoltageCurrent);
	HAL_StatusTypeDef getTorques(odrive_can_torque_t* torques);
	HAL_StatusTypeDef getPowers(odrive_can_power_t* powers);

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
