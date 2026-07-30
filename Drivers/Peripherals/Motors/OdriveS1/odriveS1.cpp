/*
 * CAN driver for the Odrive S1
 *
 * Author: Adityya Kaushal
 * Date of Creation: 2026-02-01
 */

#include "odriveS1.hpp"

#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo.h"
#include "stm32h7xx_hal_fdcan.h"
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

HAL_StatusTypeDef ODRIVES1::sendMsgCAN(uint32_t identifier, bool isRemote, const uint8_t* txBuffer) {
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

	uint8_t dummyBuffer[8] = {0};
	const uint8_t* dataPtr = txBuffer ? txBuffer : dummyBuffer;

	// Add bytes to queue to be sent
	if (HAL_FDCAN_AddMessageToTxFifoQ(this->_can, &txHeader, const_cast<uint8_t*>(dataPtr)) != HAL_OK) {
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
			memcpy(&this->heartbeat.axisError, this->odriveRxBuffer, 4);
			this->heartbeat.axisState = this->odriveRxBuffer[4];
			this->heartbeat.procedureResult = this->odriveRxBuffer[5];
			this->heartbeat.trajectoryDoneFlag = this->odriveRxBuffer[6];
			break;
		case CMD_ID_GET_ERROR:
			memcpy(&this->error.activeErrors, this->odriveRxBuffer, 4);
			memcpy(&this->error.disarmReason, &this->odriveRxBuffer[4], 4);
			break;
		case CMD_ID_GET_ENCODE_ESTIMATES:
			memcpy(&this->encoderEstimates.positionEstimate, this->odriveRxBuffer, 4);
			memcpy(&this->encoderEstimates.velocityEstimate, &this->odriveRxBuffer[4], 4);
			break;
		case CMD_ID_GET_BUS_VOLTAGE_CURRENT:
			memcpy(&this->busVoltageCurrent.busVoltage, this->odriveRxBuffer, 4);
			memcpy(&this->busVoltageCurrent.busCurrent, &this->odriveRxBuffer[4], 4);
			break;
		case CMD_ID_GET_TORQUES:
			memcpy(&this->torque.torqueTarget, this->odriveRxBuffer, 4);
			memcpy(&this->torque.torqueEstimate, &this->odriveRxBuffer[4], 4);
			break;
		case CMD_ID_GET_VERSION:
			this->version.protocolVersion = this->odriveRxBuffer[0];
			this->version.hwVersionMajor = this->odriveRxBuffer[1];
			this->version.hwVersionMinor = this->odriveRxBuffer[2];
			this->version.hwVersionVariant = this->odriveRxBuffer[3];
			this->version.fwVersionMajor = this->odriveRxBuffer[4];
			this->version.fwVersionMinor = this->odriveRxBuffer[5];
			this->version.fwVersionRevision = this->odriveRxBuffer[6];
			this->version.fwVersionUnreleased = this->odriveRxBuffer[7];
			break;
		case CMD_ID_MODIFY_PARAMETERS_RESPONSE:
			// TODO
			break;
		case CMD_ID_GET_ADDRESS:
			this->address.nodeID = this->odriveRxBuffer[0];
			memcpy((uint8_t *)&this->address.serialNumber + 2, &this->odriveRxBuffer[1], 6);
			this->address.connectionID = this->odriveRxBuffer[7];
			break;
		case CMD_ID_GET_IQ:
			memcpy(&this->iq.iqSetpoint, this->odriveRxBuffer, 4);
			memcpy(&this->iq.iqMeasured, &this->odriveRxBuffer[4], 4);
			break;
		case CMD_ID_GET_TEMPERATURE:
			memcpy(&this->temperature.FETTemperature, this->odriveRxBuffer, 4);
			memcpy(&this->temperature.motorTemperature, &this->odriveRxBuffer[4], 4);
			break;
		case CMD_ID_GET_POWERS:
			memcpy(&this->power.electricalPower, this->odriveRxBuffer, 4);
			memcpy(&this->power.mechanicalPower, &this->odriveRxBuffer[4], 4);
			break;
	}

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::setAxisState(uint32_t requestedState) {
	uint8_t txBuf[8] = {0};
	std::memcpy(&txBuf[0], &requestedState, 4);

	return this->sendMsgCAN(CMD_ID_SET_AXIS_STATE, false, txBuf);
}

HAL_StatusTypeDef ODRIVES1::setControllerMode(uint32_t controlMode, uint32_t inputMode) {
	uint8_t txBuf[8] = {0};
	std::memcpy(txBuf, &controlMode, 4);
	std::memcpy(&txBuf[4], &inputMode, 4);

	return this->sendMsgCAN(CMD_ID_SET_CONTROLLER_MODE, false, txBuf);
}

HAL_StatusTypeDef ODRIVES1::setInputPosition(float inputPos, int16_t inputVel, int16_t inputTorque) {
	uint8_t txBuf[8] = {0};
	std::memcpy(txBuf, &inputPos, 4);
	std::memcpy(&txBuf[4], &inputVel, 2);
	std::memcpy(&txBuf[6], &inputTorque, 2);

	return this->sendMsgCAN(CMD_ID_SET_INPUT_POSITION, false, txBuf);
}

HAL_StatusTypeDef ODRIVES1::setInputVelocity(float inputVel, float inputTorque) {
	uint8_t txBuf[8] = {0};
	std::memcpy(txBuf, &inputVel, 4);
	std::memcpy(&txBuf[4], &inputTorque, 4);

	return this->sendMsgCAN(CMD_ID_SET_INPUT_VELOCITY, false, txBuf);
}

HAL_StatusTypeDef ODRIVES1::setInputTorque(float inputTorque) {
	uint8_t txBuf[8] = {0};
	std::memcpy(txBuf, &inputTorque, 4);

	return this->sendMsgCAN(CMD_ID_SET_INPUT_TORQUE, false, txBuf);
}

HAL_StatusTypeDef ODRIVES1::setLimits(float velLimit, float currentSoftMax) {
	uint8_t txBuf[8] = {0};
	std::memcpy(txBuf, &velLimit, 4);
	std::memcpy(&txBuf[4], &currentSoftMax, 4);

	return this->sendMsgCAN(CMD_ID_SET_LIMITS, false, txBuf);
}

HAL_StatusTypeDef ODRIVES1::setTrajectoryVelocityLimit(float velocityLimit) {
	uint8_t txBuf[8] = {0};
	std::memcpy(txBuf, &velocityLimit, 4);

	return this->sendMsgCAN(CMD_ID_SET_TRAJECTORY_VELOCITY_LIMIT, false, txBuf);
}

HAL_StatusTypeDef ODRIVES1::setTrajectoryAccelerationLimit(float accelerationLimit, float decelerationLimit) {
	uint8_t txBuf[8] = {0};
	std::memcpy(txBuf, &accelerationLimit, 4);
	std::memcpy(&txBuf[4], &decelerationLimit, 4);

	return this->sendMsgCAN(CMD_ID_SET_TRAJECTORY_ACCELERATION_LIMIT, false, txBuf);
}

HAL_StatusTypeDef ODRIVES1::setTrajectoryInertia(float inertia) {
	uint8_t txBuf[8] = {0};
	std::memcpy(txBuf, &inertia, 4);

	return this->sendMsgCAN(CMD_ID_SET_TRAJECTORY_INERTIA, false, txBuf);
}

HAL_StatusTypeDef ODRIVES1::setAbsolutePosition(float postionEstimate) {
	uint8_t txBuf[8] = {0};
	std::memcpy(txBuf, &postionEstimate, 4);

	return this->sendMsgCAN(CMD_ID_SET_ABSOLUTE_POSITION, false, txBuf);
}

HAL_StatusTypeDef ODRIVES1::setPositionGain(float postionGain) {
	uint8_t txBuf[8] = {0};
	std::memcpy(txBuf, &postionGain, 4);

	return this->sendMsgCAN(CMD_ID_SET_POSITION_GAIN, false, txBuf);
}

HAL_StatusTypeDef ODRIVES1::setVelocityGain(float velocityGain, float velocityIntegratorGain) {
	uint8_t txBuf[8] = {0};
	std::memcpy(txBuf, &velocityGain, 4);
	std::memcpy(&txBuf[4], &velocityIntegratorGain, 4);

	return this->sendMsgCAN(CMD_ID_SET_VELOCITY_GAINS, false, txBuf);
}

HAL_StatusTypeDef ODRIVES1::modifyParameter(OpCode opCode, uint16_t endpointID, uint32_t value) {
	uint8_t txBuf[8] = {0};
	txBuf[0] = static_cast<uint8_t>(opCode);
	std::memcpy(&txBuf[1], &endpointID, 2);
	std::memcpy(&txBuf[3], &value, 4);

	return this->sendMsgCAN(CMD_ID_MODIFY_PARAMETERS, false, txBuf);
}

HAL_StatusTypeDef ODRIVES1::clearErrors(uint8_t identify) {
	uint8_t txBuf[8] = {0};
	txBuf[0] = identify;

	return this->sendMsgCAN(CMD_ID_CLEAR_ERRORS, false, txBuf);
}

HAL_StatusTypeDef ODRIVES1::rebootOdrive(ResetMode resetMode) {
	uint8_t txBuf[8] = {0};
	txBuf[0] = static_cast<uint8_t>(resetMode);

	return this->sendMsgCAN(CMD_ID_REBOOT, false, txBuf);
}

HAL_StatusTypeDef ODRIVES1::enterDFUMode() {
	uint8_t txBuf[8] = {0};

	return this->sendMsgCAN(CMD_ID_ENTER_DFU_MODE, false, txBuf);
}
