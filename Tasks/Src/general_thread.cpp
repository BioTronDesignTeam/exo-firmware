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
	BSP_LED_On(LED_RED);
	static const osThreadAttr_t init_driver_attributes = {
				.name = "InitializeDriver",
				.stack_size = 1024,
				.priority = (osPriority_t) osPriorityNormal
			};

	osThreadNew(initializeDrivers,NULL,&init_driver_attributes);

}



