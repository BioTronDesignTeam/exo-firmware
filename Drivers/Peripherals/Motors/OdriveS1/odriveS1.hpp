#pragma once
/*
 * CAN driver for the Odrive S1
 *
 * Author: Adityya Kaushal
 * Date of Creation: 2026-02-01
 */

#ifndef INC_ODRIVES1_CAN_HPP_
#define INC_ODRIVES1_CAN_HPP_

#include <cstdint>
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_fdcan.h"
#include "FreeRTOS.h"
#include "task.h"
#include "can_simple.hpp"

class ODRIVES1 {
public:
	// Must not exceed StdFiltersNbr in MX_FDCAN1_Init
	static constexpr uint8_t MAX_INSTANCES = 4;
	// Bit timing in MX_FDCAN1_Init is calculated for this kernel clock
	static constexpr uint32_t KERNEL_CLOCK_HZ = 120000000;
	static constexpr uint32_t KERNEL_CLOCK_TOLERANCE_HZ = 1000;

private:
	FDCAN_HandleTypeDef* _can;
	uint8_t _nodeId; // Must match axis0.config.can.node_id
	FDCAN_FilterTypeDef odriveCanFilter;

	static ODRIVES1* instances[MAX_INSTANCES];
	static uint8_t instanceCount;

	void handleFrame(uint32_t identifier, const uint8_t* data);

public:
	// Internal States
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

	uint32_t txErrors = 0;
	static volatile uint32_t busOffEvents;
	static volatile uint32_t rxFramesLost;


	ODRIVES1 (FDCAN_HandleTypeDef* fdcanhandle, uint8_t nodeId);

	static HAL_StatusTypeDef startBus(FDCAN_HandleTypeDef* fdcanhandle);

	static void handleRxFifo0(FDCAN_HandleTypeDef* fdcanhandle);
	static void recoverBusOff(FDCAN_HandleTypeDef* fdcanhandle);

	uint8_t nodeId() const { return _nodeId; }

	template <typename T>
	T read(const T& field) const {
		taskENTER_CRITICAL();
		T copy = field;
		taskEXIT_CRITICAL();
		return copy;
	}

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

	HAL_StatusTypeDef estop();

	// Setters
	HAL_StatusTypeDef setAxisState(AxisState requestedState);
	HAL_StatusTypeDef setControllerMode(ControlMode controlMode, InputMode inputMode);
	HAL_StatusTypeDef setInputPosition(float inputPos, float velocityFeedForward, float torqueFeedForward);
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
