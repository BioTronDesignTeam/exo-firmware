#include "BNO085.hpp"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_nucleo.h"
#include "BNO085_registers.hpp"
#include <string.h>

BNO085::BNO085(I2C_HandleTypeDef* i2cHandle):_hi2c(i2cHandle) {}

bool BNO085::begin() {

	HAL_I2C_Master_Receive(_hi2c, BNO085_I2C_ADDR, _rxBuffer, 64, HAL_MAX_DELAY);
	HAL_Delay(100);

	//verify connection by checking product ID
	uint8_t productIDPayload[] = {0xF9};

	transmitPacket(SENSOR_HUB_CONTROL, productIDPayload, 1);

	HAL_I2C_Master_Receive(_hi2c, BNO085_I2C_ADDR, _rxBuffer, 64, HAL_MAX_DELAY);

	if (_rxBuffer[4] != PRODUCT_ID_RESPONSE) {
		return false;
	}

	//enable reports
	//enableReport(ROT_VECTOR, 10000);
	return true;
}

void BNO085::enableReport(uint8_t reportID, uint32_t intervalUs) {
	//feature command bytes
	uint8_t payload[17] = {0};

	payload[0] = SET_FEATURE_COMMAND;
	payload[1] = reportID;

	//reporting interval -> converting b32 to binary
    payload[5] = intervalUs & 0xFF;
    payload[6] = (intervalUs >> 8) & 0xFF;
    payload[7] = (intervalUs >> 16) & 0xFF;
    payload[8] = (intervalUs >> 24) & 0xFF;

    transmitPacket(SENSOR_HUB_CONTROL, payload, 17);
}

void BNO085::transmitPacket(uint8_t channel, uint8_t* payload, uint8_t payloadLength) {
	//reset transmit buffer
	memset(_txBuffer, 0, sizeof(_txBuffer));

	uint16_t totalLength = payloadLength + 4;

	//packet header
	_txBuffer[0] = totalLength & 0xFF;
	_txBuffer[1] = (totalLength >> 8) & 0xFF;
	_txBuffer[2] = channel;
	_txBuffer[3] = 0x00;

	//insert into transmit buffer
	memcpy(_txBuffer+4, payload, payloadLength);

	//transmit
	HAL_I2C_Master_Transmit(_hi2c, BNO085_I2C_ADDR, _txBuffer, totalLength, 100);
}

bool BNO085::receiveReports() {
	//clear receive buffer
	memset(_rxBuffer, 0, sizeof(_rxBuffer));

	//HAL receive
	if (HAL_I2C_Master_Receive(_hi2c, BNO085_I2C_ADDR, _rxBuffer, 64, HAL_MAX_DELAY) != HAL_OK) {
		return false;
	}

	//if length is 0
	uint16_t length = (_rxBuffer[0] | (_rxBuffer[1] << 8)) & 0x7FFF;
	if (length <= 4) { //only contains header
		return false;
	}

	//proceed with parsing based on reportID
	uint8_t reportID = _rxBuffer[4];

	if (reportID == ROT_VECTOR) {
		parseRotVector();
	}

	return true;
}

void BNO085::parseRotVector() {
	float scale = 1.0f / (1 << 14); //Q point is 14
	rotationVector.i = (int16_t)(_rxBuffer[9] << 8 | _rxBuffer[8]) * scale;
	rotationVector.j = (int16_t)(_rxBuffer[11] << 8 | _rxBuffer[10]) * scale;
	rotationVector.k = (int16_t)(_rxBuffer[13] << 8 | _rxBuffer[12]) * scale;
}

