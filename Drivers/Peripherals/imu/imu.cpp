#include "imu.hpp"

#include "stm32H7xx_hal.h"
#include "stm32H7xx_hal_fdcan.h"

IMU :: IMU (I2C_HandleTypeDef *hi2c, uint8_t BNO_I2C_ADD)
{
	this-> hi2c = _hi2c;
	BNO_I2C_ADD = BNO_I2C_ADD;
	resetSequenceNumbers();

};

void IMU :: resetSequencenNumbers(){
	for(int i = 0; i<6 ; i++){
		sequenceNumbers[i]=0;
	}
};

bool IMU:: BNO_Init(){
	if(HAL_I2C_IsDeviceReady(&hi2c2, BNO_I2C_ADD, 100, 100) != HAL_OK)
		return false;
	if(!softReset())
		return false;
	HAL_Delay(100);

	return true;
};

bool IMU :: sendPacket(uint8_t channel, uint8_t *data, uint16_t length){
	uint16_t total_lenght = length +4;

	shtpHeader[0] = totalLength & 0xFF;
	shtpHeader[1] = (totalLength >> 8) & 0xFF;
	shtpHeader[2] = channel;
	shtpHeader[3] = sequenceNumbers[channel]++;

	uint8_t buffer[260];
	memcpy(buffer, shtpHeader, 4);
	memcpy(buffer + 4, data, length);
	if(HAL_I2C_Master_Transmit(&hi2c2, BNO_I2C_ADD, data, total_Length, 100) !=HAL_OK)
		return false;
	return true;
};

bool IMU :: readPacket(){
	if(HAL_I2C_Master_Receive(&hi2c2, BNO_I2C_ADD, shtpHeader , 4 , 100)!= HAL_OK)
		return false;
	packetLength =shtpHeader[0] | (shtpHeader[1]<< 8);
	packetLength -=4;
	packetChannel = shtpHeader[2];
	if(packetLength>0){
		if(HAL_I2C_Master_Receive(&hi2c2, BNO_I2C_ADD, shtpData, packetLength, 100) != HAL_OK)
		            return false;
	}
	return true;

};

