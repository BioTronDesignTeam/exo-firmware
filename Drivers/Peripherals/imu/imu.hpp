#pragma once
/*
 * IMU
 *
 * Author: Yaseman Nikoo
 * Date of Creation: 2026-02-26
 */

#include "stm32H7xx_hal.h"
#include "stm32H7xx_hal_fdcan.h"

class IMU
{
private:
	//Identifiers for SH-2 sensor feature reports used to enable and receive sensor data.
	enum class SensorReportID : uint8_t
	{
	        Accelerometer = 0x01,
	        Gyroscope = 0x02,
			Magnetic_Field =0x03,
			Linear_Acceleration =0x04,
			Rotation_Vector=0x05,
			Gravity=0x06,
			Uncalibrated_Gyroscope =0x07,
			Game_Rotation_Vector=0x08,
			Geomagnetic_Rotation_Vector = 0x09,
			Ucalibrated_Magnetic_Feild = 0x0F,
			Tap_Detector = 0x010,
			Step_Counter = 0x11,
			Significatn_Motion = 0x12,
			Stability_Classifier = 0x13,
			Raw_Accelerometer = 0x14,
			Raw_Gyroscope = 0x15,
			Raw_Magnetometer = 0x16,
			SAR = 0x17,
			Step_Detector = 0x18,
			Shake_Detector = 0x19,
			Flip_Detector = 0x1A,
			Pickup_Detector = 0x1B,
			Stablility_Detector = 0x1C,
			Personal_Activity_Classifier = 0x1E,
			Sleep_Detector = 0x1F,
			Tilt_Detector = 0x20,
			Pocket_Detector = 0x21,
			Circle_Detector = 0x22,
			Hear_Rate_Monitor = 0x23,
			ARVR_Stabilized_Rotation_vector= 0x28,
			ARVR_Stabilized_Game_Rotaton_Vector = 0x29
	    };
	//Identifiers for SH-2 command messages used to configure or query the sensor.
	enum class CommandReportID : uint8_t
	{
		Get_Feature_Request = 0xFE,
		Set_Feature_Command = 0xFD,
		Get_Feature_Response = 0xFC,
		Base_Timestamp = 0xFB,
		Timestamp_Rebase = 0xFA,
		Product_ID_Request = 0xF9,
		Product_ID_Response = 0xF8,
		FRS_Write_Request = 0xF7,
		FRS_Write_Date = 0xF6,
		FRS_Write_Response = 0xF5,
		FRS_Read_Reqest = 0xF4,
		FRS_Read_Response = 0xF3,
		Command_Request = 0xF2,
		Command_Response = 0xF1
	};
	//Flash Record System identifiers used to access configuration data stored in non-volatile memory.
	enum class FRSReportId : uint16_t
	{
		Static_Calibration_AGM = 0x7979,
		Nominal_Calibration_AGM = 0x4D4D,
		Static_Calibration_SRA = 0x8A8A,
		Nominal_Calibration_SRA = 0x4E4E,
		Dynamic_Calibration = 0x1F1F,
		MotionEngine_Power_Managment = 0xD3E2,
		System_Orientation = 0x2D3E,
		Primary_Accelerometer_Orientation = 0x2D41,
		Gyroscope_Orientation = 0x2D4C,
		Magnetometer_Orientation = 0x2D4C,
		ARVR_Stabilization_Rotation_Vector = 0x3E2D,
		ARVR_Stabilization_Game_Rotation_Vector = 0x3E2E,
		Significant_Motion_Detector_Config = 0xC274,
		Shake_Detector_Config = 0x7D7D,
		Maximum_Fusion_Period = 0xD7D6,
		Serial_Number = 0x4B4B,
		Environmental_Pressure_Calibration = 0x39AF,
		Environmental_Temperature_Calibration = 0x4D20,
		Environmental_Humidity_Calibation = 0x1AC9,
		Environmental_Ambient_Light_Calibration = 0x39B1,
		Environmental_Proximity_Calibraton = 0x4DA2,
		ALS_Calibration = 0xD401,
		Proximity_Sensor_Calibration = 0xD402,
		Stability_Detector_Config = 0xED85,
		User_Record = 0x74B4,
		MotionEngine_Time_Source_Selection = 0xD403,
		Gyro_Integrated_Rotation_Vector_config = 0xA1A2

	};


	//SHTP channels
	enum ShtpChannel : uint8_t
	{
		Command_Channel = 0,
		Executable_Channel = 1,
		SHC_Channel = 2,
		Input_Sensor_Report = 3,
		Wake_Input_Sensor_Report = 4,
		Gyro_Rotation_Vector = 5

	};

	enum class Sh2CommandId : uint8_t
	{
	    Errors = 1,
	    CommandCounter = 2,
	    Tare = 3,
	    Initialize = 4,
		Reserved = 5,
	    Dcd = 6,
	    Me_Cal = 7,
		DCD_Save = 9,
		Oscillator = 10,
		Clear_DCD_Reset = 11,
		Calibration = 12,
		Bootloader = 13,
		Interactive_Calibration = 14
	};

	I2C_HandleTypeDef *_hi2c;
	uint8_t BNO_I2C_ADD = 0x4B;
	uint8_t DFU_I2C_ADD =0x29; // idk if we need this?

	bool sendPacket(uint8_t channel, uint8_t *data, uint16_t length);
	bool readPacket();
    void resetSequenceNumbers();

    uint8_t sequenceNumber[6];

    uint8_t shtpHeader[4];
    uint8_t shtpData[256];

    uint16_t packetLength;
    uint8_t packetChannel;

	// Raw sensor storage
    int16_t rawQuatI, rawQuatJ, rawQuatK, rawQuatReal;
    int16_t rawAccelX, rawAccelY, rawAccelZ;

	public:
	IMU(I2C_HandleTypeDef *hi2c, uint8_t BNO_I2C_ADD);

	// Runtime functions
	bool dataAvailable();
	bool parsePacket(); //idk maybe this should be in private part?

	//set up functions
	bool BNO_Init();
	bool BNO_SoftReset();
	bool enableRotationVector(uint32_t interval_us);
	bool enableAccelerometer(uint32_t interval_us);

	//Getters
	float getQuatI();
	float getQuatJ();
	float getQuatK();
	float getQuatReal();

	float getAccelX();
	float getAccelY();
	float getAccelZ();

	//setters
};

