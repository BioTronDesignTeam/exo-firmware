#include "general_thread.hpp"
#include "drivers.hpp"
#include "uart.hpp"



extern "C" void initTasks() {
	init_uart_tasks();
	//motorControllerInitTask();
}


void show_imu_success(void* arg) {

	for ( ;; ) {
		if (bno085Handle->begin()) {
			BSP_LED_Toggle(LED_GREEN);
		} else {
			BSP_LED_Toggle(LED_RED);
		}
		osDelay(500);
	}
}
extern "C" void initDrivers() {
	//from drivers.hpp, directly inside main.c will cause issues
	initializeDrivers();
	osThreadId_t test_IMU_task_handle;


	static const osThreadAttr_t test_IMU_task_attributes = {
		.name = "SendTelemetrytoESP",
		.stack_size = 1024,
		.priority = (osPriority_t) osPriorityNormal
	};
	test_IMU_task_handle = osThreadNew(show_imu_success, NULL, &test_IMU_task_attributes);
}


