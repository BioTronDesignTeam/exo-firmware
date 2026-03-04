#include "imu.hpp"

#include "stm32H7xx_hal.h"
#include "stm32H7xx_hal_fdcan.h"
#include <cstring>


IMU :: IMU (I2C_HandleTypeDef *hi2c, uint8_t BNO_I2C_ADD)
{
	this-> _hi2c = hi2c;
	this->BNO_I2C_ADD= BNO_I2C_ADD;
	resetSequenceNumbers();

};

void IMU :: resetSequenceNumbers(){
	for(int i = 0; i<6 ; i++){
		sequenceNumber[i]=0;
	}
};

bool IMU:: BNO_Init(){
	if(HAL_I2C_IsDeviceReady(_hi2c, BNO_I2C_ADD, 100, 100) != HAL_OK)
		return false;
	if(! BNO_SoftReset())
		return false;
	HAL_Delay(100);

	return true;
};

bool IMU :: sendPacket(uint8_t channel, uint8_t *data, uint16_t length){
	uint16_t totalLength = length +4;

	shtpHeader[0] = totalLength & 0xFF;
	shtpHeader[1] = (totalLength >> 8) & 0xFF;
	shtpHeader[2] = channel;
	shtpHeader[3] = sequenceNumber[channel]++;

	uint8_t buffer[260];
	memcpy(buffer, shtpHeader, 4);
	memcpy(buffer + 4, data, length);
	if(HAL_I2C_Master_Transmit(_hi2c, BNO_I2C_ADD, data, totalLength, 100) !=HAL_OK)
		return false;
	return true;
};

bool IMU :: readPacket(){
	if(HAL_I2C_Master_Receive(_hi2c, BNO_I2C_ADD, shtpHeader , 4 , 100)!= HAL_OK)
		return false;
	packetLength =shtpHeader[0] | (shtpHeader[1]<< 8);
	packetLength -=4;
	packetChannel = shtpHeader[2];
	if(packetLength>0){
		if(HAL_I2C_Master_Receive(_hi2c, BNO_I2C_ADD, shtpData, packetLength, 100) != HAL_OK)
		            return false;
	}
	return true;

};

bool IMU::BNO_SoftReset(){
	uint8_t payload[1];
	payload[0] = static_cast<uint8_t>(Sh2CommandId:: Clear_DCD_Reset);
	return sendPacket(static_cast<uint8_t>(ShtpChannel::Command_Channel), payload, 1);

}

