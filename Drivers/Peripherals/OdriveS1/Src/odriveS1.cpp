/*
 * CAN driver for the Odrive S1
 *
 * Author: Adityya Kaushal
 * Date of Creation: 2026-02-01
 */

#include "odrives1.hpp"

#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo.h"
#include "stm32H7xx_hal_fdcan.h"
#include "can_simple.hpp"
#include <algorithm>
#include <cstring>

ODRIVES1::ODRIVES1 (FDCAN_HandleTypeDef* fdcanhandle) : _can(fdcanhandle) {
	// TODO: Add Error Handling
	// Configure Filter
	this->odriveCanFilter.IdType = FDCAN_STANDARD_ID;
	this->odriveCanFilter.FilterIndex = 0;
	// Set our filter to mask so it uses ID1 as a value and ID2 as mask
	this->odriveCanFilter.FilterType = FDCAN_FILTER_MASK;
	this->odriveCanFilter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	// Assume node_id of Odrive is 0
	this->odriveCanFilter.FilterID1 = 0x000;
	// Mask first five bits (16 bit mask, 11 bit CAN address)
	this->odriveCanFilter.FilterID2 = 0b11111 << 10;
	this->odriveCanFilter.RxBufferIndex = 0;

	if (HAL_FDCAN_ConfigFilter(this->_can, &this->odriveCanFilter) != HAL_OK) {
		BSP_LED_On(LED_RED);
	}

	if (HAL_FDCAN_Start(this->_can) != HAL_OK) {
		BSP_LED_On(LED_RED);
	}

	if (HAL_FDCAN_ActivateNotification(this->_can, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) {
		BSP_LED_On(LED_RED);
	}
}

HAL_StatusTypeDef ODRIVES1::sendMsgCAN(uint32_t identifier, bool isRemote) {
	FDCAN_TxHeaderTypeDef txHeader;
	txHeader.Identifier = identifier;
	txHeader.IdType = FDCAN_STANDARD_ID;
	if (isRemote) {
		txHeader.TxFrameType = FDCAN_REMOTE_FRAME;
	} else {
		txHeader.TxFrameType = FDCAN_DATA_FRAME;
	}
	txHeader.DataLength = FDCAN_DLC_BYTES_8;
	txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	txHeader.BitRateSwitch = FDCAN_BRS_OFF;
	txHeader.FDFormat = FDCAN_CLASSIC_CAN;
	txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	txHeader.MessageMarker = 0x00; // Ignore because FDCAN_NO_TX_EVENTS

	// Add bytes to queue to be sent
	if (HAL_FDCAN_AddMessageToTxFifoQ(this->_can, &txHeader, this->odriveTxBuffer) != HAL_OK) {
		return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::getVersion() {
	return this->sendMsgCAN(CMD_ID_GET_VERSION, true);
}

HAL_StatusTypeDef ODRIVES1::getHeartbeat() {
	return this->sendMsgCAN(CMD_ID_GET_HEARTBEAT, true);
}

HAL_StatusTypeDef ODRIVES1::getError() {
	return this->sendMsgCAN(CMD_ID_GET_ERROR, true);
}

HAL_StatusTypeDef ODRIVES1::getCANAddress() {
	return this->sendMsgCAN(CMD_ID_GET_ADDRESS, true);
}

HAL_StatusTypeDef ODRIVES1::getEncoderEstimates() {
	return this->sendMsgCAN(CMD_ID_GET_ENCODE_ESTIMATES, true);
}

HAL_StatusTypeDef ODRIVES1::getIq() {
	return this->sendMsgCAN(CMD_ID_GET_IQ, true);
}

HAL_StatusTypeDef ODRIVES1::getTemperatures() {
	return this->sendMsgCAN(CMD_ID_GET_TEMPERATURE, true);
}

HAL_StatusTypeDef ODRIVES1::getBusVoltageCurrent() {
	return this->sendMsgCAN(CMD_ID_GET_BUS_VOLTAGE_CURRENT, true);
}

HAL_StatusTypeDef ODRIVES1::getTorques() {
	return this->sendMsgCAN(CMD_ID_GET_TORQUES, true);
}

HAL_StatusTypeDef ODRIVES1::getPowers() {
	return this->sendMsgCAN(CMD_ID_GET_POWERS, true);
}

HAL_StatusTypeDef ODRIVES1::responseCallback(uint32_t identifier) {
	switch (identifier) {
		// The messages are encoded in little endian
		case CMD_ID_GET_HEARTBEAT:
			this->heartbeat.axisError = (uint32_t)(this->odriveRxBuffer[3] << 24 | this->odriveRxBuffer[2] << 16 | this->odriveRxBuffer[1] << 8 | this->odriveRxBuffer[0]);
			this->heartbeat.axisState = this->odriveRxBuffer[4];
			this->heartbeat.procedureResult = this->odriveRxBuffer[5];
			this->heartbeat.trajectoryDoneFlag = this->odriveRxBuffer[6];
		case CMD_ID_GET_ERROR:
			this->error.activeErrors = (uint32_t)(this->odriveRxBuffer[3] << 24 | this->odriveRxBuffer[2] << 16 | this->odriveRxBuffer[1] << 8 | this->odriveRxBuffer[0]);
			this->error.disarmReason = (uint32_t)(this->odriveRxBuffer[7] << 24 | this->odriveRxBuffer[6] << 16 | this->odriveRxBuffer[5] << 8 | this->odriveRxBuffer[4]);
		case CMD_ID_GET_ENCODE_ESTIMATES:
			this->encoderEstimates.positionEstimate = (float)(this->odriveRxBuffer[3] << 24 | this->odriveRxBuffer[2] << 16 | this->odriveRxBuffer[1] << 8 | this->odriveRxBuffer[0]);
			this->encoderEstimates.velocityEstimate = (float)(this->odriveRxBuffer[7] << 24 | this->odriveRxBuffer[6] << 16 | this->odriveRxBuffer[5] << 8 | this->odriveRxBuffer[4]);
		case CMD_ID_GET_BUS_VOLTAGE_CURRENT:
			this->busVoltageCurrent.busVoltage = (float)(this->odriveRxBuffer[3] << 24 | this->odriveRxBuffer[2] << 16 | this->odriveRxBuffer[1] << 8 | this->odriveRxBuffer[0]);
			this->busVoltageCurrent.busCurrent = (float)(this->odriveRxBuffer[7] << 24 | this->odriveRxBuffer[6] << 16 | this->odriveRxBuffer[5] << 8 | this->odriveRxBuffer[4]);
		case CMD_ID_GET_TORQUES:
			this->torque.torqueTarget = (float)(this->odriveRxBuffer[3] << 24 | this->odriveRxBuffer[2] << 16 | this->odriveRxBuffer[1] << 8 | this->odriveRxBuffer[0]);
			this->torque.torqueEstimate = (float)(this->odriveRxBuffer[7] << 24 | this->odriveRxBuffer[6] << 16 | this->odriveRxBuffer[5] << 8 | this->odriveRxBuffer[4]);
		case CMD_ID_GET_VERSION:
			this->version.protocolVersion = this->odriveRxBuffer[0];
			this->version.hwVersionMajor = this->odriveRxBuffer[1];
			this->version.hwVersionMinor = this->odriveRxBuffer[2];
			this->version.hwVersionVariant = this->odriveRxBuffer[3];
			this->version.fwVersionMajor = this->odriveRxBuffer[4];
			this->version.fwVersionMinor = this->odriveRxBuffer[5];
			this->version.fwVersionRevision = this->odriveRxBuffer[6];
			this->version.fwVersionUnreleased = this->odriveRxBuffer[7];
		case CMD_ID_MODIFY_PARAMETERS_RESPONSE:
			// TODO
		case CMD_ID_GET_ADDRESS:
			this->address.nodeID = this->odriveRxBuffer[0];
			this->address.serialNumber = (uint64_t)(this->odriveRxBuffer[5] << 32 | this->odriveRxBuffer[4] << 24 | this->odriveRxBuffer[3] << 16 | this->odriveRxBuffer[2] << 8 | this->odriveRxBuffer[1]);
			this->address.connectionID = this->odriveRxBuffer[6];
		case CMD_ID_GET_IQ:
			this->iq.iqSetpoint = (float)(this->odriveRxBuffer[3] << 24 | this->odriveRxBuffer[2] << 16 | this->odriveRxBuffer[1] << 8 | this->odriveRxBuffer[0]);
			this->iq.iqMeasured = (float)(this->odriveRxBuffer[7] << 24 | this->odriveRxBuffer[6] << 16 | this->odriveRxBuffer[5] << 8 | this->odriveRxBuffer[4]);
		case CMD_ID_GET_TEMPERATURE:
			this->temperature.FETTemperature = (float)(this->odriveRxBuffer[3] << 24 | this->odriveRxBuffer[2] << 16 | this->odriveRxBuffer[1] << 8 | this->odriveRxBuffer[0]);
			this->temperature.motorTemperature = (float)(this->odriveRxBuffer[7] << 24 | this->odriveRxBuffer[6] << 16 | this->odriveRxBuffer[5] << 8 | this->odriveRxBuffer[4]);
		case CMD_ID_GET_POWERS:
			this->power.electricalPower = (float)(this->odriveRxBuffer[3] << 24 | this->odriveRxBuffer[2] << 16 | this->odriveRxBuffer[1] << 8 | this->odriveRxBuffer[0]);
			this->power.mechanicalPower = (float)(this->odriveRxBuffer[7] << 24 | this->odriveRxBuffer[6] << 16 | this->odriveRxBuffer[5] << 8 | this->odriveRxBuffer[4]);
	}

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::setAxisState(uint32_t requestedState) {
	std::memcpy(&this->odriveTxBuffer[0], &requestedState, 4);


	return this->sendMsgCAN(CMD_ID_SET_AXIS_STATE, false);
}

HAL_StatusTypeDef ODRIVES1::setControllerMode(uint32_t controlMode, uint32_t inputMode) {
	std::memcpy(this->odriveTxBuffer, &controlMode, 4);
	std::memcpy(&this->odriveTxBuffer[4], &inputMode, 4);

	return this->sendMsgCAN(CMD_ID_SET_CONTROLLER_MODE, false);
}

HAL_StatusTypeDef ODRIVES1::setInputPosition(float inputPos, int16_t inputVel, int16_t inputTorque) {
	std::memcpy(this->odriveTxBuffer, &inputPos, 4);
	std::memcpy(&this->odriveTxBuffer[4], &inputVel, 2);
	std::memcpy(&this->odriveTxBuffer[6], &inputTorque, 2);

	return this->sendMsgCAN(CMD_ID_SET_INPUT_POSITION, false);
}

HAL_StatusTypeDef ODRIVES1::setInputVelocity(float inputVel, float inputTorque) {
	std::memcpy(this->odriveTxBuffer, &inputVel, 4);
	std::memcpy(&this->odriveTxBuffer[4], &inputTorque, 4);

	return this->sendMsgCAN(CMD_ID_SET_INPUT_VELOCITY, false);
}

HAL_StatusTypeDef ODRIVES1::setInputTorque(float inputTorque) {
	std::memcpy(this->odriveTxBuffer, &inputTorque, 4);

	return this->sendMsgCAN(CMD_ID_SET_INPUT_TORQUE, false);
}

HAL_StatusTypeDef ODRIVES1::setLimits(float velLimit, float currentSoftMax) {
	std::memcpy(this->odriveTxBuffer, &velLimit, 4);
	std::memcpy(&this->odriveTxBuffer[4], &currentSoftMax, 4);

	return this->sendMsgCAN(CMD_ID_SET_LIMITS, false);
}

HAL_StatusTypeDef ODRIVES1::setTrajectoryVelocityLimit(float velocityLimit) {
	std::memcpy(this->odriveTxBuffer, &velocityLimit, 4);

	return this->sendMsgCAN(CMD_ID_SET_TRAJECTORY_VELOCITY_LIMIT, false);
}

HAL_StatusTypeDef ODRIVES1::setTrajectoryAccelerationLimit(float accelerationLimit, float decelerationLimit) {
	std::memcpy(this->odriveTxBuffer, &accelerationLimit, 4);
	std::memcpy(&this->odriveTxBuffer[4], &decelerationLimit, 4);

	return this->sendMsgCAN(CMD_ID_SET_TRAJECTORY_ACCELERATION_LIMIT, false);
}

HAL_StatusTypeDef ODRIVES1::setTrajectoryInertia(float inertia) {
	std::memcpy(this->odriveTxBuffer, &inertia, 4);

	return this->sendMsgCAN(CMD_ID_SET_TRAJECTORY_INERTIA, false);
}

HAL_StatusTypeDef ODRIVES1::setAbsolutePosition(float postionEstimate) {
	std::memcpy(this->odriveTxBuffer, &postionEstimate, 4);

	return this->sendMsgCAN(CMD_ID_SET_ABSOLUTE_POSITION, false);
}

HAL_StatusTypeDef ODRIVES1::setPositionGain(float postionGain) {
	std::memcpy(this->odriveTxBuffer, &postionGain, 4);

	return this->sendMsgCAN(CMD_ID_SET_POSITION_GAIN, false);
}

HAL_StatusTypeDef ODRIVES1::setVelocityGain(float velocityGain, float velocityIntegratorGain) {
	std::memcpy(this->odriveTxBuffer, &velocityGain, 4);
	std::memcpy(&this->odriveTxBuffer[4], &velocityIntegratorGain, 4);

	return this->sendMsgCAN(CMD_ID_SET_VELOCITY_GAINS, false);
}

HAL_StatusTypeDef ODRIVES1::modifyParameter(OpCode opCode, uint16_t endpointID, uint32_t value) {
	this->odriveTxBuffer[0] = static_cast<uint8_t>(opCode);
	std::memcpy(&this->odriveTxBuffer[1], &endpointID, 2);
	std::memcpy(&this->odriveTxBuffer[3], &value, 4);

	return this->sendMsgCAN(CMD_ID_MODIFY_PARAMETERS, false);
}

HAL_StatusTypeDef ODRIVES1::clearErrors(uint8_t identify) {
	this->odriveTxBuffer[0] = identify;

	return this->sendMsgCAN(CMD_ID_CLEAR_ERRORS, false);
}

HAL_StatusTypeDef ODRIVES1::rebootOdrive(ResetMode resetMode) {
	this->odriveTxBuffer[0] = static_cast<uint8_t>(resetMode);

	return this->sendMsgCAN(CMD_ID_REBOOT, false);
}

HAL_StatusTypeDef ODRIVES1::enterDFUMode() {
	std::memset(this->odriveTxBuffer, 0, 8);

	return this->sendMsgCAN(CMD_ID_ENTER_DFU_MODE, false);
}
