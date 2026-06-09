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


static const char *TAG = "UART_SIM";
const uart_port_t uart_stm = UART_NUM_2;

enum UART_STATE {
    WAIT_AA,
    WAIT_55,
    READ_DATA
};

void uart_init() {
    //             uart_num, rx_buffer_size, tx_buffer_size, queue_size, queue_handle, flags
    ESP_ERROR_CHECK(uart_driver_install(uart_stm, 1024, 1024, 0, NULL, 0));
    
    uart_config_t uart_config = {
        .baud_rate = 921600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(uart_stm, &uart_config));
    
    ESP_ERROR_CHECK(uart_set_pin(uart_stm, 17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));


    //make sure esp32 can actually match the baud rate
    uint32_t actual_baud;
    uart_get_baudrate(uart_stm, &actual_baud);
    ESP_LOGI("UART", "Requested: 921600, Actual: %lu", actual_baud);
}

telemetry_data_t read_telemetry_from_uart() {
    telemetry_data_t data;

    enum UART_STATE state = WAIT_AA;
    while (true) {
        switch (state) {
            case WAIT_AA:
                uint8_t byte;
                uart_read_bytes(uart_stm, &byte, 1, portMAX_DELAY);
                if (byte == 0xAA) {
                    state = WAIT_55;
                }
                break;

            case WAIT_55:
                uint8_t byte;
                uart_read_bytes(uart_stm, &byte, 1, portMAX_DELAY);
                if (byte == 0x55) {
                    state = READ_DATA;
                } else {
                    state = WAIT_AA; // reset if not correct
                }
                break;

            case READ_DATA:
                /*uint32_t data_length;
                uart_read_bytes(uart_stm, &data_length, 1, portMAX_DELAY);
                if (data_length != telemetry_data_size) { // expecting 3 uint32_t = 12 bytes
                    ESP_LOGW(TAG, "Unexpected data length: %lu", data_length);
                    state = WAIT_AA; // reset if length is wrong
                    break;
                }
                    */

                
                uart_read_bytes(uart_stm, &data, sizeof(data), portMAX_DELAY);
                //check crc
                
                return data; // return after reading one set of data
        }
    }
    return data;
}


