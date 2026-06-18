#include "general_thread.hpp"
#include <cstring>
#include "drivers.hpp"


extern "C" {

#include "main.h"

}

extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart2;

void spamUART(void* arg) {
	uint64_t count = 0;

	for ( ;; ) {

		char tx[] = "hello";
		char rx[16];

		char example[64];
		snprintf(example, sizeof(example), "printing to uart: %llu" , count);

		HAL_StatusTypeDef err = HAL_UART_Transmit(&huart5, (uint8_t*)example, strlen(example),HAL_MAX_DELAY);

		count++;

		if (err == HAL_OK) {
			BSP_LED_Toggle(LED_YELLOW);
		}
		if (err == HAL_ERROR) {
			BSP_LED_Toggle(LED_GREEN);
		}
		if (err == HAL_TIMEOUT) {
			BSP_LED_Toggle(LED_RED);
		}
		osDelay(500);
	}
}

extern "C" void initTasks() {

	//motorControllerInitTask();

	osThreadId_t spamUARTHandle;

	static const osThreadAttr_t spamUARTAttributes = {
	    .name = "SpamUART",
	    .stack_size = 1024,
	    .priority = (osPriority_t) osPriorityNormal
	};
	spamUARTHandle = osThreadNew(spamUART, NULL, &spamUARTAttributes);


}

extern "C" void initDrivers() {
	//from drivers.hpp, directly inside main.c will cause issues
	initializeDrivers();

}



