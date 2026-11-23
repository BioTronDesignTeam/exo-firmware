#include "general_thread.hpp"
#include "drivers.hpp"
#include "uart.hpp"
#include "imu.hpp"



extern "C" void initTasks() {
	init_uart_tasks();
	init_imu_tasks();
	//motorControllerInitTask();
}

// One-shot init thread: sets up the drivers, then creates the worker tasks
// that depend on them, then exits. Because the workers don't exist until
// setup is done, start-up order doesn't depend on task priorities.
static void initThread(void* arg) {
	(void)arg;
	BSP_LED_On(LED_RED);
	constructDrivers();
	initTasks();
	BSP_LED_Off(LED_RED);
	osThreadExit();
}

extern "C" void initDrivers() {
	//from drivers.hpp, directly inside main.c will cause issues
	static const osThreadAttr_t init_driver_attributes = {
				.name = "InitializeDriver",
				.stack_size = 1024,
				.priority = (osPriority_t) osPriorityHigh
			};

	osThreadNew(initThread,NULL,&init_driver_attributes);

}



