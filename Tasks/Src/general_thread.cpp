#include "general_thread.hpp"
#include <cstring>
#include "drivers.hpp"


extern "C" {

#include "main.h"

}

extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart2;

extern "C" void initTasks() {

	motorControllerInitTask();


	uint8_t example[] = "wow new line \n";
	HAL_UART_Transmit(&huart4, example, strlen((char*)example), HAL_MAX_DELAY);

}

extern "C" void initDrivers() {
	//from drivers.hpp, directly inside main.c will cause issues
	initializeDrivers();

}

