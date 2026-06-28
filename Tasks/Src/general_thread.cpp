#include "general_thread.hpp"
#include "drivers.hpp"
#include "uart.hpp"



extern "C" void initTasks() {
	init_uart_tasks();
	//motorControllerInitTask();
}

extern "C" void initDrivers() {
	//from drivers.hpp, directly inside main.c will cause issues
	initializeDrivers();

}



