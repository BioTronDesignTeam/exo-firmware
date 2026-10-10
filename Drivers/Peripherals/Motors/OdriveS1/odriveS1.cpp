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
#include <cmath>
#include <cstdint>
#include <cstring>

ODRIVES1* ODRIVES1::instances[ODRIVES1::MAX_INSTANCES] = {nullptr};
uint8_t ODRIVES1::instanceCount = 0;
volatile uint32_t ODRIVES1::busOffEvents = 0;
volatile uint32_t ODRIVES1::rxFramesLost = 0;

ODRIVES1::ODRIVES1 (FDCAN_HandleTypeDef* fdcanhandle, uint8_t nodeId) : _can(fdcanhandle), _nodeId(nodeId & 0x3F) {
	bool canRegister = instanceCount < MAX_INSTANCES;
	for (uint8_t i = 0; i < instanceCount; ++i) {
		if (instances[i]->_can == this->_can && instances[i]->_nodeId == this->_nodeId) {
			canRegister = false;
		}
	}
	if (!canRegister) {
		BSP_LED_On(LED_RED);
		return;
	}

	// Configure Filter
	this->odriveCanFilter.IdType = FDCAN_STANDARD_ID;
	this->odriveCanFilter.FilterIndex = instanceCount;
	// Set our filter to mask so it uses ID1 as a value and ID2 as mask
	this->odriveCanFilter.FilterType = FDCAN_FILTER_MASK;
	this->odriveCanFilter.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
	this->odriveCanFilter.FilterID1 = static_cast<uint32_t>(_nodeId) << 5;
	this->odriveCanFilter.FilterID2 = 0x7E0;
	this->odriveCanFilter.RxBufferIndex = 0;

	if (HAL_FDCAN_ConfigFilter(this->_can, &this->odriveCanFilter) != HAL_OK) {
		BSP_LED_On(LED_RED);
		return;
	}

	instances[instanceCount] = this;
	++instanceCount;
}

HAL_StatusTypeDef ODRIVES1::startBus(FDCAN_HandleTypeDef* fdcanhandle) {
	const uint32_t kernelClock = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_FDCAN);
	if (kernelClock < KERNEL_CLOCK_HZ - KERNEL_CLOCK_TOLERANCE_HZ ||
	    kernelClock > KERNEL_CLOCK_HZ + KERNEL_CLOCK_TOLERANCE_HZ) {
		return HAL_ERROR;
	}

	if (HAL_FDCAN_ConfigGlobalFilter(fdcanhandle, FDCAN_REJECT, FDCAN_REJECT,
	                                 FDCAN_REJECT_REMOTE, FDCAN_REJECT_REMOTE) != HAL_OK) {
		return HAL_ERROR;
	}

	if (HAL_FDCAN_Start(fdcanhandle) != HAL_OK) {
		return HAL_ERROR;
	}

	return HAL_FDCAN_ActivateNotification(fdcanhandle,
	                                      FDCAN_IT_RX_FIFO0_NEW_MESSAGE | FDCAN_IT_RX_FIFO0_MESSAGE_LOST |
	                                      FDCAN_IT_BUS_OFF, 0);
}

void ODRIVES1::recoverBusOff(FDCAN_HandleTypeDef* fdcanhandle) {
	FDCAN_ProtocolStatusTypeDef status;
	if (HAL_FDCAN_GetProtocolStatus(fdcanhandle, &status) != HAL_OK || status.BusOff == 0U) {
		return;
	}

	++busOffEvents;
	// Leaving INIT starts the bus-off recovery sequence (129 x 11 recessive bits)
	CLEAR_BIT(fdcanhandle->Instance->CCCR, FDCAN_CCCR_INIT);
}

void ODRIVES1::handleRxFifo0(FDCAN_HandleTypeDef* fdcanhandle) {
	while (HAL_FDCAN_GetRxFifoFillLevel(fdcanhandle, FDCAN_RX_FIFO0) > 0) {
		FDCAN_RxHeaderTypeDef rxHeader;
		uint8_t rxData[8] = {0};
		if (HAL_FDCAN_GetRxMessage(fdcanhandle, FDCAN_RX_FIFO0, &rxHeader, rxData) != HAL_OK) {
			BSP_LED_On(LED_RED);
			return;
		}

		const uint8_t node = static_cast<uint8_t>(rxHeader.Identifier >> 5);
		for (uint8_t i = 0; i < instanceCount; ++i) {
			if (instances[i]->_can == fdcanhandle && instances[i]->_nodeId == node) {
				instances[i]->handleFrame(rxHeader.Identifier, rxData);
				break;
			}
		}
	}
}

HAL_StatusTypeDef ODRIVES1::sendMsgCAN(uint32_t identifier, bool isRemote, const uint8_t* txBuffer) {
	FDCAN_TxHeaderTypeDef txHeader;
	txHeader.Identifier = (static_cast<uint32_t>(_nodeId) << 5) | (identifier & 0x1F);
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
		++txErrors;
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

HAL_StatusTypeDef ODRIVES1::estop() {
	return this->sendMsgCAN(CMD_ID_ESTOP, false);
}

void ODRIVES1::handleFrame(uint32_t identifier, const uint8_t* data) {
	switch (identifier & 0x1F) {
		// The messages are encoded in little endian
		case CMD_ID_GET_HEARTBEAT:
			memcpy(&this->heartbeat.axisError, data, 4);
			this->heartbeat.axisState = data[4];
			this->heartbeat.procedureResult = data[5];
			this->heartbeat.trajectoryDoneFlag = data[6];
			break;
		case CMD_ID_GET_ERROR:
			memcpy(&this->error.activeErrors, data, 4);
			memcpy(&this->error.disarmReason, &data[4], 4);
			break;
		case CMD_ID_GET_ENCODE_ESTIMATES:
			memcpy(&this->encoderEstimates.positionEstimate, data, 4);
			memcpy(&this->encoderEstimates.velocityEstimate, &data[4], 4);
			break;
		case CMD_ID_GET_BUS_VOLTAGE_CURRENT:
			memcpy(&this->busVoltageCurrent.busVoltage, data, 4);
			memcpy(&this->busVoltageCurrent.busCurrent, &data[4], 4);
			break;
		case CMD_ID_GET_TORQUES:
			memcpy(&this->torque.torqueTarget, data, 4);
			memcpy(&this->torque.torqueEstimate, &data[4], 4);
			break;
		case CMD_ID_GET_VERSION:
			this->version.protocolVersion = data[0];
			this->version.hwVersionMajor = data[1];
			this->version.hwVersionMinor = data[2];
			this->version.hwVersionVariant = data[3];
			this->version.fwVersionMajor = data[4];
			this->version.fwVersionMinor = data[5];
			this->version.fwVersionRevision = data[6];
			this->version.fwVersionUnreleased = data[7];
			break;
		case CMD_ID_MODIFY_PARAMETERS_RESPONSE:
			memcpy(&this->latestEndpointChange.endpointId, &data[1], 2);
			memcpy(&this->latestEndpointChange.value, &data[4], 4);
			break;
		case CMD_ID_GET_ADDRESS:
			this->address.nodeID = data[0];
			this->address.serialNumber = 0;
			memcpy(&this->address.serialNumber, &data[1], 6);
			this->address.connectionID = data[7];
			break;
		case CMD_ID_GET_IQ:
			memcpy(&this->iq.iqSetpoint, data, 4);
			memcpy(&this->iq.iqMeasured, &data[4], 4);
			break;
		case CMD_ID_GET_TEMPERATURE:
			memcpy(&this->temperature.FETTemperature, data, 4);
			memcpy(&this->temperature.motorTemperature, &data[4], 4);
			break;
		case CMD_ID_GET_POWERS:
			memcpy(&this->power.electricalPower, data, 4);
			memcpy(&this->power.mechanicalPower, &data[4], 4);
			break;
	}
}

HAL_StatusTypeDef ODRIVES1::setAxisState(AxisState requestedState) {
	uint8_t txBuf[8] = {0};
	const uint32_t state = static_cast<uint32_t>(requestedState);
	std::memcpy(&txBuf[0], &state, 4);

	return this->sendMsgCAN(CMD_ID_SET_AXIS_STATE, false, txBuf);
}

HAL_StatusTypeDef ODRIVES1::setControllerMode(ControlMode controlMode, InputMode inputMode) {
	uint8_t txBuf[8] = {0};
	const uint32_t control = static_cast<uint32_t>(controlMode);
	const uint32_t input = static_cast<uint32_t>(inputMode);
	std::memcpy(txBuf, &control, 4);
	std::memcpy(&txBuf[4], &input, 4);

	return this->sendMsgCAN(CMD_ID_SET_CONTROLLER_MODE, false, txBuf);
}

// Set_Input_Pos sends feed-forwards as int16 in units of 0.001 rev/s and 0.001 Nm
static int16_t toFeedForward(float value) {
	const long scaled = std::lround(value * 1000.0f);
	return static_cast<int16_t>(std::max<long>(INT16_MIN, std::min<long>(scaled, INT16_MAX)));
}

HAL_StatusTypeDef ODRIVES1::setInputPosition(float inputPos, float velocityFeedForward, float torqueFeedForward) {
	uint8_t txBuf[8] = {0};
	const int16_t velocity = toFeedForward(velocityFeedForward);
	const int16_t torque = toFeedForward(torqueFeedForward);
	std::memcpy(txBuf, &inputPos, 4);
	std::memcpy(&txBuf[4], &velocity, 2);
	std::memcpy(&txBuf[6], &torque, 2);

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
	std::memcpy(&txBuf[4], &value, 4);

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
