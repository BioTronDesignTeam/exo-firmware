#include "general_thread.hpp"
#include "drivers.hpp"
#include "uart.hpp"
#include "imu.hpp"



extern "C" void initTasks() {
	//init_uart_tasks(); 
	init_imu();
	//motorControllerInitTask();
}

extern "C" void initDrivers() {
	//from drivers.hpp, directly inside main.c will cause issues
	initializeDrivers();

}



