#pragma once

#include "odriveS1.hpp"
#include "mpu6050.hpp"
#include "msa311.hpp"
#include "bno085.hpp"

extern ODRIVES1 *odriveS1Handle;
// extern MPU6050 *MPU6050Handle;
extern MSA311 *MSA311Handle;
extern BNO085 *bno085Handle;

void initializeDrivers();
