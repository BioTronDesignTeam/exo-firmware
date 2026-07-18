#include "general_thread.hpp"
#include "drivers.hpp"
#include "uart.hpp"
#include "BNO085_registers.hpp"

extern "C" {

#include "main.h"

}

extern I2C_HandleTypeDef hi2c1;
extern BNO085 *bno085Handle;


extern "C" void initTasks() {
	//init_uart_tasks();
	//motorControllerInitTask();
}


void show_imu_success(void* arg) {
	bool works = bno085Handle->begin();
	for ( ;; ) {
		if (HAL_I2C_IsDeviceReady(&hi2c1, BNO085_I2C_ADDR, 1, 10) == HAL_OK) {
			BSP_LED_Toggle(LED_YELLOW);
		} else {
			BSP_LED_Toggle(LED_RED);
		}
		if (works) {
			BSP_LED_Toggle(LED_GREEN);
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


