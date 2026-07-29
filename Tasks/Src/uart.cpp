#include <cstring>
#include "uart.hpp"
#include <cmsis_os2.h>
#include "imu.hpp"
#include "drivers.hpp"
#include "rtos_objects.hpp"
//uart2: estop
//uart4: to esp
//uart5: to jetson or rpi


extern UART_HandleTypeDef huart7;
extern UART_HandleTypeDef huart2;
extern QueueHandle_t esp_print_queue;


void empty_print_queue_to_esp(void* arg) {
    log_message_t msg;

    for (;;) {
        // Block indefinitely until a message arrives
        if (xQueueReceive(esp_print_queue, &msg, portMAX_DELAY) == pdPASS) {
            HAL_UART_Transmit(&huart2, msg.data, msg.len, 100);
            BSP_LED_Toggle(LED_RED);
        }
    }
}
HAL_StatusTypeDef esp_print(uint8_t* str, uint16_t len) {
    log_message_t msg;
    if (len > LOG_MSG_MAX_LEN) len = LOG_MSG_MAX_LEN; // truncate or reject
    memcpy(msg.data, str, len);
    msg.len = len;

    if (xQueueSend(esp_print_queue, &msg, pdMS_TO_TICKS(20)) != pdPASS) {
        return HAL_ERROR; // queue full — dropped
    }
    return HAL_OK;
}
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


telemetry_data_t get_telemetry_data() {
    telemetry_data_t data = {0};
    data.example1 = 1;
    data.example2 = 2; 
    data.example3 = 3;
    // if (MPU6050Handle == nullptr) {
    //     return data;
    // }

    // data.mpu6050_data.accel_x = MPU6050Handle->mpu6050_data.accel_x;
    // data.mpu6050_data.accel_y = MPU6050Handle->mpu6050_data.accel_y;
    // data.mpu6050_data.accel_z = MPU6050Handle->mpu6050_data.accel_z;
    // data.mpu6050_data.gyro_x  = MPU6050Handle->mpu6050_data.gyro_x;
    // data.mpu6050_data.gyro_y  = MPU6050Handle->mpu6050_data.gyro_y;
    // data.mpu6050_data.gyro_z  = MPU6050Handle->mpu6050_data.gyro_z;
    // data.mpu6050_data.temp = MPU6050Handle->mpu6050_data.temp;

    return data;
}

void write_to_esp(telemetry_data_t data) {
	telemetry_packet_t packet;
	packet.header[0] = 0xAA;
	packet.header[1] = 0x55;
	memcpy (&packet.data, &data, sizeof(data));

	packet.crc = crc16((uint8_t*)&packet.data, sizeof(telemetry_data_t));

	HAL_StatusTypeDef err = HAL_UART_Transmit(&huart7, (uint8_t*)&packet, sizeof(telemetry_packet_t), HAL_MAX_DELAY);
    //esp_print((uint8_t*)"Sent telemetry data to ESP\r\n", 24);
    
	if (err == HAL_OK) {
        BSP_LED_Toggle(LED_GREEN);
	}
	if (err == HAL_ERROR) {
		// BSP_LED_Toggle(LED_RED);
	}
	
	osDelay(500);
}

void send_telemetry_to_esp(void* arg) { //to esp32

	for ( ;; ) {
		telemetry_data_t data = get_telemetry_data();
		write_to_esp(data);
		osDelay(500); //change this probably
	}

}

void init_uart_tasks() {
	osThreadId_t send_telemetry_to_esp_handle;
	osThreadId_t empty_print_queue_to_esp_handle;

	static const osThreadAttr_t send_telemetry_to_esp_attributes = {
		.name = "SendTelemetrytoESP",
		.stack_size = 1024,
		.priority = (osPriority_t) osPriorityNormal
	};
	static const osThreadAttr_t empty_print_queue_to_esp_attributes = {
			.name = "EmptyPrintQueueToESP",
			.stack_size = 1024,
			.priority = (osPriority_t) osPriorityNormal
		};
	send_telemetry_to_esp_handle = osThreadNew(send_telemetry_to_esp, NULL, &send_telemetry_to_esp_attributes);
	empty_print_queue_to_esp_handle = osThreadNew(empty_print_queue_to_esp, NULL, &empty_print_queue_to_esp_attributes);

}

enum UART_STATE {
    WAIT_AA,
    WAIT_55,
    READ_DATA
};

void read_telemetry_from_uart_esp() { //placeholder, need to figure out command structure being sent from esp
	//this code can probably be repurposed into the receiver on the jetson
	uint8_t MAGIC_BYTE_1 = 0xAA;
	uint8_t MAGIC_BYTE_2 = 0x55;
    enum UART_STATE state = WAIT_AA;
    while (true) {
        uint8_t byte;
        switch (state) {
            case WAIT_AA:
                HAL_UART_Receive(&huart7, &byte, 1, HAL_MAX_DELAY);
                if (byte == MAGIC_BYTE_1) {
                    state = WAIT_55;
                }
            break;

            case WAIT_55:
                HAL_UART_Receive(&huart7, &byte, 1, HAL_MAX_DELAY);
                if (byte == MAGIC_BYTE_2) {
                    state = READ_DATA;
                } else {
                    state = WAIT_AA; // reset if not correct
                }
            break;

            case READ_DATA:


                telemetry_data_t data;
                HAL_UART_Receive(&huart7, (uint8_t*)&data, sizeof(data), HAL_MAX_DELAY);
                uint16_t received_crc;
                HAL_UART_Receive(&huart7, (uint8_t*)&received_crc, sizeof(received_crc), HAL_MAX_DELAY);

                //check crc
                uint16_t computed_crc = crc16((uint8_t*)&data, sizeof(data)); //crc16((uint8_t*)&packet,
                if (received_crc != computed_crc) {
                    state = WAIT_AA; // reset if crc is wrong
                    break;
                }
                // do something with the data here
                state = WAIT_AA; // reset for next packet
            break;
        }
    }
}
