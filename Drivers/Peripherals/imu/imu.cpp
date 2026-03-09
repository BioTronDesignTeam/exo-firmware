#include "imu.hpp"

#include "stm32H7xx_hal.h"
#include "stm32H7xx_hal_fdcan.h"
#include <cstring>


IMU :: IMU (I2C_HandleTypeDef *hi2c)
{
	this-> _hi2c = hi2c;
	resetSequenceNumbers();

}

void IMU :: resetSequenceNumbers(){
	for(int i = 0; i<6 ; i++){
		sequenceNumber[i]=0;
	}
}

bool IMU:: BNO_Init(){
	if(HAL_I2C_IsDeviceReady(_hi2c, BNO_I2C_ADD, 100, 100) != HAL_OK)
		return false;
	if(! BNO_SoftReset())
		return false;
	HAL_Delay(100);

	return true;
}

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
}

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

}

bool IMU::BNO_SoftReset(){
	uint8_t payload[1];
	payload[0] = static_cast<uint8_t>(Sh2CommandId:: Clear_DCD_Reset);
	return sendPacket(static_cast<uint8_t>(ShtpChannel::Command_Channel), payload, 1);

}

bool IMU:: enableRotationVector(uint32_t interval){
	uint8_t payload[17]= {0};
	payload[0] = static_cast<uint8_t> (CommandReportID:: Set_Feature_Command);
    payload[1] = static_cast<uint8_t> (SensorReportID::Rotation_Vector);   // Rotation Vector report ID
    payload[5] = interval & 0xFF;
    payload[6] = (interval >> 8) & 0xFF;
    payload[7] = (interval >> 16) & 0xFF;
    payload[8] = (interval >> 24) & 0xFF;
	return sendPacket(static_cast<uint8_t>(ShtpChannel::SHC_Channel), payload, 17);
}

bool IMU::enableAccelerometer(uint32_t interval){
	uint8_t payload[17] = {0};

	payload[0]= static_cast<uint8_t> (CommandReportID::Set_Feature_Command);
	payload[1] = static_cast<uint8_t> (SensorReportID::Accelerometer);
	payload[6] = (interval >> 8) & 0xFF;
    payload[7] = (interval >> 16) & 0xFF;
    payload[8] = (interval >> 24) & 0xFF;

}

bool IMU::dataAvailable()
{
    return readPacket();
}

bool IMU::parsePacket()
{
    if(packetChannel != 3)
        return false;

    uint8_t reportID = shtpData[0];

    if(reportID == static_cast<uint8_t>(SensorReportID::Rotation_Vector))
    {
        rawQuatI    = (int16_t)(shtpData[4]  | (shtpData[5] << 8));
        rawQuatJ    = (int16_t)(shtpData[6]  | (shtpData[7] << 8));
        rawQuatK    = (int16_t)(shtpData[8]  | (shtpData[9] << 8));
        rawQuatReal = (int16_t)(shtpData[10] | (shtpData[11] << 8));
        return true;
    }

    if(reportID == static_cast<uint8_t>(SensorReportID::Accelerometer))
    {
        rawAccelX = (int16_t)(shtpData[4] | (shtpData[5] << 8));
        rawAccelY = (int16_t)(shtpData[6] | (shtpData[7] << 8));
        rawAccelZ = (int16_t)(shtpData[8] | (shtpData[9] << 8));
        return true;
    }

    return false;
}

float IMU::getQuatI()    { return rawQuatI / 16384.0f; }
float IMU::getQuatJ()    { return rawQuatJ / 16384.0f; }
float IMU::getQuatK()    { return rawQuatK / 16384.0f; }
float IMU::getQuatReal() { return rawQuatReal / 16384.0f; }

float IMU::getAccelX() { return rawAccelX / 256.0f; }
float IMU::getAccelY() { return rawAccelY / 256.0f; }
float IMU::getAccelZ() { return rawAccelZ / 256.0f; }





