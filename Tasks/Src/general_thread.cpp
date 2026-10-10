#include "general_thread.hpp"
#include "drivers.hpp"
#include "uart.hpp"
#include "imu.hpp"

static void startup(void* arg) {
	init_uart_tasks();
	init_imu_tasks();
	//motorControllerInitTask();

	static const osThreadAttr_t init_driver_attributes = {
				.name = "InitializeDriver",
				.stack_size = 1024,
				.priority = (osPriority_t) osPriorityNormal
			};
	osThreadNew(initializeDrivers, NULL, &init_driver_attributes);

	osThreadExit();
}

extern "C" void initTasks() {
	static const osThreadAttr_t startup_attributes = {
				.name = "Startup",
				.stack_size = 1024,
				.priority = (osPriority_t) osPriorityHigh
			};
	osThreadNew(startup, NULL, &startup_attributes);
}
