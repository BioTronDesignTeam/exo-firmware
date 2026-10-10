#include "general_thread.hpp"
#include "drivers.hpp"
#include "uart.hpp"
#include "imu.hpp"
#include "safety.hpp"
#include "task_priorities.hpp"

static void startup(void* arg) {
	if (!safety_init()) {
		BSP_LED_On(LED_RED);
	}
	init_uart_tasks();
	init_imu_tasks();
	//motorControllerInitTask();

	static const osThreadAttr_t init_driver_attributes = {
				.name = "InitializeDriver",
				.stack_size = 1024,
				.priority = TaskPriority::DriverInit
			};
	osThreadNew(initializeDrivers, NULL, &init_driver_attributes);

	osThreadExit();
}

extern "C" void initTasks() {
	static const osThreadAttr_t startup_attributes = {
				.name = "Startup",
				.stack_size = 1024,
				.priority = TaskPriority::Startup
			};
	osThreadNew(startup, NULL, &startup_attributes);
}
