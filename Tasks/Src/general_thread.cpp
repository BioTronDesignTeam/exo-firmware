#include "general_thread.hpp"
#include "drivers.hpp"
#include "uart.hpp"
#include "imu.hpp"



extern "C" void initTasks() {
	init_uart_tasks();
	init_imu_tasks();
	//motorControllerInitTask();
}

extern "C" void initDrivers() {
	//from drivers.hpp, directly inside main.c will cause issues
	//ESP_PRINTLN("drivers initialized");
	initializeDrivers();
	//bno085Handle->begin();
}



