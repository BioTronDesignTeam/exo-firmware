/*
 * CAN driver for the Odrive S1
 *
 * Author: Adityya Kaushal
 * Date of Creation: 2026-02-01
 */

#include "odrives1.hpp"

#include "stm32H7xx_hal.h"
#include "stm32H7xx_hal_fdcan.h"
#include "CAN_simple.hpp"
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

	HAL_FDCAN_ConfigFilter(this->_can, &this->odriveCanFilter);

	HAL_FDCAN_Start(this->_can);
	HAL_FDCAN_ActivateNotification(this->_can, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
}

HAL_StatusTypeDef ODRIVES1::sendGetMsgCAN(uint32_t identifier) {
	FDCAN_TxHeaderTypeDef txHeader;
	txHeader.Identifier = identifier;
	txHeader.IdType = FDCAN_STANDARD_ID;
	txHeader.TxFrameType = FDCAN_REMOTE_FRAME;
	txHeader.DataLength = FDCAN_DLC_BYTES_8;
	txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
	txHeader.BitRateSwitch = FDCAN_BRS_OFF;
	txHeader.FDFormat = FDCAN_CLASSIC_CAN;
	txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
	txHeader.MessageMarker = 0x00; // Ignore because FDCAN_NO_TX_EVENTS

	// Add bytes to queue to be sent
	HAL_FDCAN_AddMessageToTxFifoQ(this->_can, &txHeader, this->odriveTxBuffer);

	// Wait for a message to come in
	while(HAL_FDCAN_GetRxFifoFillLevel(this->_can, FDCAN_RX_FIFO0) < 1) {
	}

	// Retrieve elements from FIFO queue
	if (HAL_FDCAN_GetRxMessage(this->_can, FDCAN_RX_FIFO0, &this->odriveCanRxHeader, this->odriveRxBuffer) != HAL_OK) {
		return HAL_ERROR;
	}

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::sendSetMsgCAN(uint32_t identifier) {
	FDCAN_TxHeaderTypeDef txHeader;
	txHeader.Identifier = identifier;
	txHeader.IdType = FDCAN_STANDARD_ID;
	txHeader.TxFrameType = FDCAN_DATA_FRAME;
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

HAL_StatusTypeDef ODRIVES1::getVersion(odrive_can_version_t* version) {
	this->sendGetMsgCAN(CMD_ID_GET_VERSION);

	version->protocolVersion = this->odriveRxBuffer[0];
	version->hwVersionMajor = this->odriveRxBuffer[1];
	version->hwVersionMinor = this->odriveRxBuffer[2];
	version->hwVersionVariant = this->odriveRxBuffer[3];
	version->fwVersionMajor = this->odriveRxBuffer[4];
	version->fwVersionMinor = this->odriveRxBuffer[5];
	version->fwVersionRevision = this->odriveRxBuffer[6];
	version->fwVersionUnreleased = this->odriveRxBuffer[7];

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::getHeartbeat(odrive_can_heartbeat_t* heartbeat) {
	this->sendGetMsgCAN(CMD_ID_GET_HEARTBEAT);

	// The messages are encoded in little endian
	heartbeat->axisError = (uint32_t)(this->odriveRxBuffer[3] << 24 | this->odriveRxBuffer[2] << 16 | this->odriveRxBuffer[1] << 8 | this->odriveRxBuffer[0]);
	heartbeat->axisState = this->odriveRxBuffer[4];
	heartbeat->procedureResult = this->odriveRxBuffer[5];
	heartbeat->trajectoryDoneFlag = this->odriveRxBuffer[6];

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::getError(odrive_can_error_t* error) {
	this->sendGetMsgCAN(CMD_ID_GET_ERROR);

	// The messages are encoded in little endian
	error->activeErrors = (uint32_t)(this->odriveRxBuffer[3] << 24 | this->odriveRxBuffer[2] << 16 | this->odriveRxBuffer[1] << 8 | this->odriveRxBuffer[0]);
	error->disarmReason = (uint32_t)(this->odriveRxBuffer[7] << 24 | this->odriveRxBuffer[6] << 16 | this->odriveRxBuffer[5] << 8 | this->odriveRxBuffer[4]);


	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::getCANAddress(odrive_can_address_t* address) {
	this->sendGetMsgCAN(CMD_ID_GET_ADDRESS);

	// The messages are encoded in little endian
	address->nodeID = this->odriveRxBuffer[0];
	address->serialNumber = (uint64_t)(this->odriveRxBuffer[5] << 32 | this->odriveRxBuffer[4] << 24 | this->odriveRxBuffer[3] << 16 | this->odriveRxBuffer[2] << 8 | this->odriveRxBuffer[1]);
	address->connectionID = this->odriveRxBuffer[6];


	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::getEncoderEstimates(odrive_can_encoder_estimates_t* encoderEstimates) {
	this->sendGetMsgCAN(CMD_ID_GET_ENCODE_ESTIMATES);

	// The messages are encoded in little endian
	encoderEstimates->positionEstimate = (float)(this->odriveRxBuffer[3] << 24 | this->odriveRxBuffer[2] << 16 | this->odriveRxBuffer[1] << 8 | this->odriveRxBuffer[0]);
	encoderEstimates->velocityEstimate = (float)(this->odriveRxBuffer[7] << 24 | this->odriveRxBuffer[6] << 16 | this->odriveRxBuffer[5] << 8 | this->odriveRxBuffer[4]);


	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::getIq(odrive_can_iq_t* iq) {
	this->sendGetMsgCAN(CMD_ID_GET_IQ);

	// The messages are encoded in little endian
	iq->iqSetpoint = (float)(this->odriveRxBuffer[3] << 24 | this->odriveRxBuffer[2] << 16 | this->odriveRxBuffer[1] << 8 | this->odriveRxBuffer[0]);
	iq->iqMeasured = (float)(this->odriveRxBuffer[7] << 24 | this->odriveRxBuffer[6] << 16 | this->odriveRxBuffer[5] << 8 | this->odriveRxBuffer[4]);


	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::getTemperatures(odrive_can_temperature_t* temperature) {
	this->sendGetMsgCAN(CMD_ID_GET_TEMPERATURE);

	// The messages are encoded in little endian
	temperature->FETTemperature = (float)(this->odriveRxBuffer[3] << 24 | this->odriveRxBuffer[2] << 16 | this->odriveRxBuffer[1] << 8 | this->odriveRxBuffer[0]);
	temperature->motorTemperature = (float)(this->odriveRxBuffer[7] << 24 | this->odriveRxBuffer[6] << 16 | this->odriveRxBuffer[5] << 8 | this->odriveRxBuffer[4]);


	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::getBusVoltageCurrent(odrive_can_bus_t* busVoltageCurrent) {
	this->sendGetMsgCAN(CMD_ID_GET_BUS_VOLTAGE_CURRENT);

	// The messages are encoded in little endian
	busVoltageCurrent->busVoltage = (float)(this->odriveRxBuffer[3] << 24 | this->odriveRxBuffer[2] << 16 | this->odriveRxBuffer[1] << 8 | this->odriveRxBuffer[0]);
	busVoltageCurrent->busCurrent = (float)(this->odriveRxBuffer[7] << 24 | this->odriveRxBuffer[6] << 16 | this->odriveRxBuffer[5] << 8 | this->odriveRxBuffer[4]);


	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::getTorques(odrive_can_torque_t* torques) {
	this->sendGetMsgCAN(CMD_ID_GET_TORQUES);

	// The messages are encoded in little endian
	torques->torqueTarget = (float)(this->odriveRxBuffer[3] << 24 | this->odriveRxBuffer[2] << 16 | this->odriveRxBuffer[1] << 8 | this->odriveRxBuffer[0]);
	torques->torqueEstimate = (float)(this->odriveRxBuffer[7] << 24 | this->odriveRxBuffer[6] << 16 | this->odriveRxBuffer[5] << 8 | this->odriveRxBuffer[4]);


	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::getPowers(odrive_can_power_t* powers) {
	this->sendGetMsgCAN(CMD_ID_GET_POWERS);

	// The messages are encoded in little endian
	powers->electricalPower = (float)(this->odriveRxBuffer[3] << 24 | this->odriveRxBuffer[2] << 16 | this->odriveRxBuffer[1] << 8 | this->odriveRxBuffer[0]);
	powers->mechanicalPower = (float)(this->odriveRxBuffer[7] << 24 | this->odriveRxBuffer[6] << 16 | this->odriveRxBuffer[5] << 8 | this->odriveRxBuffer[4]);


	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::setAxisState(uint32_t requestedState) {
	std::memcpy(&this->odriveTxBuffer[4], &requestedState, 4);
	std::reverse(this->odriveTxBuffer, this->odriveTxBuffer + 8);


	this->sendSetMsgCAN(CMD_ID_SET_AXIS_STATE);

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::setControllerMode(uint32_t controlMode, uint32_t inputMode) {
	std::memcpy(this->odriveTxBuffer, &inputMode, 4);
	std::memcpy(&this->odriveTxBuffer[4], &controlMode, 4);
	std::reverse(this->odriveTxBuffer, this->odriveTxBuffer + 8);

	this->sendSetMsgCAN(CMD_ID_SET_CONTROLLER_MODE);

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::setInputPosition(float inputPos, int16_t inputVel, int16_t inputTorque) {
	std::memcpy(this->odriveTxBuffer, &inputTorque, 2);
	std::memcpy(&this->odriveTxBuffer[2], &inputVel, 2);
	std::memcpy(&this->odriveTxBuffer[4], &inputPos, 4);
	std::reverse(this->odriveTxBuffer, this->odriveTxBuffer + 8);

	this->sendSetMsgCAN(CMD_ID_SET_INPUT_POSITION);

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::setInputVelocity(float inputVel, float inputTorque) {
	std::memcpy(this->odriveTxBuffer, &inputTorque, 4);
	std::memcpy(&this->odriveTxBuffer[4], &inputVel, 4);
	std::reverse(this->odriveTxBuffer, this->odriveTxBuffer + 8);

	this->sendSetMsgCAN(CMD_ID_SET_INPUT_VELOCITY);

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::setInputTorque(float inputTorque) {
	std::memcpy(&this->odriveTxBuffer[4], &inputTorque, 4);
	std::reverse(this->odriveTxBuffer, this->odriveTxBuffer + 8);

	this->sendSetMsgCAN(CMD_ID_SET_INPUT_TORQUE);

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::setLimits(float velLimit, float currentSoftMax) {
	std::memcpy(this->odriveTxBuffer, &currentSoftMax, 4);
	std::memcpy(&this->odriveTxBuffer[4], &velLimit, 4);
	std::reverse(this->odriveTxBuffer, this->odriveTxBuffer + 8);

	this->sendSetMsgCAN(CMD_ID_SET_LIMITS);

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::setTrajectoryVelocityLimit(float velocityLimit) {
	std::memcpy(&this->odriveTxBuffer[4], &velocityLimit, 4);
	std::reverse(this->odriveTxBuffer, this->odriveTxBuffer + 8);

	this->sendSetMsgCAN(CMD_ID_SET_TRAJECTORY_VELOCITY_LIMIT);

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::setTrajectoryAccelerationLimit(float accelerationLimit, float decelerationLimit) {
	std::memcpy(this->odriveTxBuffer, &decelerationLimit, 4);
	std::memcpy(&this->odriveTxBuffer[4], &accelerationLimit, 4);
	std::reverse(this->odriveTxBuffer, this->odriveTxBuffer + 8);

	this->sendSetMsgCAN(CMD_ID_SET_TRAJECTORY_ACCELERATION_LIMIT);

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::setTrajectoryInertia(float inertia) {
	std::memcpy(&this->odriveTxBuffer[4], &inertia, 4);
	std::reverse(this->odriveTxBuffer, this->odriveTxBuffer + 8);

	this->sendSetMsgCAN(CMD_ID_SET_TRAJECTORY_INERTIA);

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::setAbsolutePosition(float postionEstimate) {
	std::memcpy(&this->odriveTxBuffer[4], &postionEstimate, 4);
	std::reverse(this->odriveTxBuffer, this->odriveTxBuffer + 8);

	this->sendSetMsgCAN(CMD_ID_SET_ABSOLUTE_POSITION);

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::setPositionGain(float postionGain) {
	std::memcpy(&this->odriveTxBuffer[4], &postionGain, 4);
	std::reverse(this->odriveTxBuffer, this->odriveTxBuffer + 8);

	this->sendSetMsgCAN(CMD_ID_SET_POSITION_GAIN);

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::setVelocityGain(float velocityGain, float velocityIntegratorGain) {
	std::memcpy(this->odriveTxBuffer, &velocityIntegratorGain, 4);
	std::memcpy(&this->odriveTxBuffer[4], &velocityGain, 4);
	std::reverse(this->odriveTxBuffer, this->odriveTxBuffer + 8);

	this->sendSetMsgCAN(CMD_ID_SET_VELOCITY_GAINS);

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::modifyParameter(OpCode opCode, uint16_t endpointID, uint32_t value) {
	std::memcpy(this->odriveTxBuffer, &value, 4);
	std::memcpy(&this->odriveTxBuffer[5], &endpointID, 2);
	this->odriveTxBuffer[7] = static_cast<uint8_t>(opCode);
	std::reverse(this->odriveTxBuffer, this->odriveTxBuffer + 8);

	this->sendSetMsgCAN(CMD_ID_MODIFY_PARAMETERS);

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::clearErrors(uint8_t identify) {
	this->odriveTxBuffer[0] = identify;

	this->sendSetMsgCAN(CMD_ID_CLEAR_ERRORS);

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::rebootOdrive(ResetMode resetMode) {
	this->odriveTxBuffer[0] = static_cast<uint8_t>(resetMode);

	this->sendSetMsgCAN(CMD_ID_REBOOT);

	return HAL_OK;
}

HAL_StatusTypeDef ODRIVES1::enterDFUMode() {
	std::memset(this->odriveTxBuffer, 0, 8);
	this->sendSetMsgCAN(CMD_ID_ENTER_DFU_MODE);

	return HAL_OK;
}
