//written for chip ESP32-D0WD-V3
//board esp32 devkit v1
//put the wires on rx2 and tx2
//gpio 16 and 17

//message format:
//0xAA 0x55 [data length] [data] [crc]
#include "handle_uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "crc.h"
#include "esp_task_wdt.h"

#define MAGIC_BYTE_1 0xAA
#define MAGIC_BYTE_2 0x55

#define UART_TIMEOUT_MS  100   // tune to your baud rate / packet rate


static const char *TAG = "HANDLE_UART";
const uart_port_t uart_stm = UART_NUM_2;
const uart_port_t uart_print = UART_NUM_1;

enum UART_STATE {
    WAIT_AA,
    WAIT_55,
    READ_DATA
};

void uart_init() {
    //             uart_num, rx_buffer_size, tx_buffer_size, queue_size, queue_handle, flags
    ESP_ERROR_CHECK(uart_driver_install(uart_stm, 1024, 1024, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_driver_install(uart_print, 1024, 1024, 0, NULL, 0));

    uart_config_t uart_config_stm = {
        .baud_rate = 921600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_config_t uart_config_print = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(uart_stm  , &uart_config_stm));
    ESP_ERROR_CHECK(uart_param_config(uart_print, &uart_config_print));
    //                                     //tx rx
    ESP_ERROR_CHECK(uart_set_pin(uart_stm,   17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_set_pin(uart_print, 19, 18, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));


    //make sure esp32 can actually match the baud rate
    uint32_t actual_baud;
    uart_get_baudrate(uart_stm, &actual_baud);
    ESP_LOGI("UART STM", "Requested: 921600, Actual: %lu", actual_baud);
    uart_get_baudrate(uart_print, &actual_baud);
    ESP_LOGI("UART PRINT", "Requested: 115200, Actual: %lu", actual_baud);
}


//TODO:
//change this to the command structure with the enum, according receive function in the stm code Tasks/Src/uart.cpp is also commented out with a similar todo
/*
void write_data_to_stm (telemetry_data_t data) {
    telemetry_packet_t packet;
    packet.header[0] = MAGIC_BYTE_1;
    packet.header[1] = MAGIC_BYTE_2;
    memcpy(&packet.data, &data, sizeof(data));

    packet.crc = crc16((uint8_t*)&packet, offsetof(telemetry_packet_t, crc));

    uart_write_bytes(uart_stm, (const char*)&packet, sizeof(packet));
}
    */

void stm_print() {
    uint8_t buffer[1024];
    while (true) {
        int bytes_read = uart_read_bytes(uart_print, buffer, sizeof(buffer) - 1, pdMS_TO_TICKS(UART_TIMEOUT_MS));
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0'; // Null-terminate the string
            ESP_LOGI(TAG, "STM: length %d, data: %s", bytes_read, buffer);
        }
        
    }
}

void read_telemetry_from_uart() {
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = 5000,
        .idle_core_mask = 0,
        .trigger_panic = true,
    };
    esp_task_wdt_reconfigure(&wdt_config);
    esp_task_wdt_add(NULL);
    enum UART_STATE state = WAIT_AA;

    while (true) {
        esp_task_wdt_reset();   // feed watchdog while idle
        uint8_t byte;
        int bytes_read;
        uint8_t data_buffer[sizeof(telemetry_packet_t)];

        switch (state) {
            
            case WAIT_AA:
                bytes_read = uart_read_bytes(uart_stm, &byte, 1, pdMS_TO_TICKS(UART_TIMEOUT_MS));
                if (bytes_read > 0 && byte == MAGIC_BYTE_1) state = WAIT_55;
            break;
                    
            case WAIT_55:
                bytes_read = uart_read_bytes(uart_stm, &byte, 1, pdMS_TO_TICKS(UART_TIMEOUT_MS));
                if (bytes_read <= 0 || byte != MAGIC_BYTE_2) state = WAIT_AA;
                else state = READ_DATA;
            break;

            case READ_DATA:
                ESP_LOGI(TAG, "Reading telemetry data...");
                telemetry_data_t data;

                bytes_read = uart_read_bytes(uart_stm, (uint8_t*)&data,
                                             sizeof(data),
                                             pdMS_TO_TICKS(UART_TIMEOUT_MS));
                if (bytes_read < (int)sizeof(data)) {
                    ESP_LOGW(TAG, "Short read on data payload");
                    esp_task_wdt_reset();
                    state = WAIT_AA;
                    break;
                }

                uint16_t received_crc;
                bytes_read = uart_read_bytes(uart_stm, (uint8_t*)&received_crc,
                                             sizeof(received_crc),
                                             pdMS_TO_TICKS(UART_TIMEOUT_MS));
                if (bytes_read < (int)sizeof(received_crc)) {
                    ESP_LOGW(TAG, "Short read on CRC");
                    esp_task_wdt_reset();
                    state = WAIT_AA;
                    break;
                }

                uint16_t computed_crc = crc16((uint8_t*)&data, sizeof(data));
                if (received_crc != computed_crc) {
                    ESP_LOGW(TAG, "CRC mismatch: expected %04X, got %04X",
                             computed_crc, received_crc);
                    esp_task_wdt_reset();
                    state = WAIT_AA;
                    break;
                }

                ESP_LOGI(TAG, "Received telemetry: example1=%lu example2=%lu example3=%lu",
                         data.example1, data.example2, data.example3);
                esp_task_wdt_reset();
                state = WAIT_AA;
            break;
        }
    }
}

