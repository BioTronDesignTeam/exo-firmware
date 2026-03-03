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
	//Report ID for the sensors use in SH-2
	enum class SensorReport : uint8_t
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
	enum class Control_Mssg : uint8_t
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


};
