#include <cstring>
#include "uart.hpp"
#include <cmsis_os2.h>

extern "C" {

#include "main.h"

}

extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart5;
extern UART_HandleTypeDef huart2;

uint16_t crc16(uint8_t *data, uint32_t length)
{
    uint16_t crc = 0x0000;
    for (uint32_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8;  // bring byte into high bits
        for (int bit = 0; bit < 8; bit++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;  // CCITT poly
            else
                crc = (crc << 1);
        }
    }
    return crc;
}

void spamUART(void* arg) {
	uint64_t count = 0;

	for ( ;; ) {
		char example[64];
		snprintf(example, sizeof(example), "printing to uart: %llu" , count);

		HAL_StatusTypeDef err = HAL_UART_Transmit(&huart5, (uint8_t*)example, strlen(example), HAL_MAX_DELAY);

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

#include <stdlib.h>
telemetry_data_t get_telemetry_data() { //placeholder, get real ones from odrive
	srand(12932);
	telemetry_data_t sample_data = {rand(), rand(), rand()};

	return sample_data;
}

void write_to_esp(telemetry_data_t data) {
	telemetry_packet_t packet;
	packet.header[0] = 0xAA;
	packet.header[1] = 0x55;
	packet.data_length = sizeof(telemetry_data_t);
	memcpy (&packet.data, &data, sizeof(data));

	packet.crc = crc16((uint8_t*)&packet, offsetof(telemetry_packet_t, crc));

	HAL_UART_Transmit(&huart4, (uint8_t*)&packet, sizeof(telemetry_packet_t), HAL_MAX_DELAY);
}

void send_telemetry_to_esp(void* arg) {

	for ( ;; ) {
		telemetry_data_t data = get_telemetry_data();
		write_to_esp(data);
		osDelay(500); //change this probably
	}

}


void init_uart_tasks() {
	osThreadId_t spamUARTHandle;
	osThreadId_t send_telemetry_to_esp_handle;

	static const osThreadAttr_t spamUARTAttributes = {
	    .name = "SpamUART",
	    .stack_size = 1024,
	    .priority = (osPriority_t) osPriorityNormal
	};
	static const osThreadAttr_t send_telemetry_to_esp_attributes = {
		.name = "SendTelemetrytoESP",
		.stack_size = 1024,
		.priority = (osPriority_t) osPriorityNormal
	};
	spamUARTHandle = osThreadNew(spamUART, NULL, &spamUARTAttributes);
	send_telemetry_to_esp_handle = osThreadNew(send_telemetry_to_esp, NULL, &send_telemetry_to_esp_attributes);

}
