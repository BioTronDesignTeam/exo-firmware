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

#define MAGIC_BYTE_1 0xAA
#define MAGIC_BYTE_2 0x55

static const char *TAG = "UART_SIM";
const uart_port_t uart_stm = UART_NUM_2;
const uart_port_t uart_estop = UART_NUM_1;

enum UART_STATE {
    WAIT_AA,
    WAIT_55,
    READ_DATA
};

void uart_init() {
    //             uart_num, rx_buffer_size, tx_buffer_size, queue_size, queue_handle, flags
    ESP_ERROR_CHECK(uart_driver_install(uart_stm, 1024, 1024, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_driver_install(uart_estop, 1024, 1024, 0, NULL, 0));

    uart_config_t uart_config_stm = {
        .baud_rate = 921600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    uart_config_t uart_config_estop = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };
    
    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(uart_stm  , &uart_config_stm));
    ESP_ERROR_CHECK(uart_param_config(uart_estop, &uart_config_estop));
    //                                     //tx rx
    ESP_ERROR_CHECK(uart_set_pin(uart_stm,   17, 16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_set_pin(uart_estop, 19, 18, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));


    //make sure esp32 can actually match the baud rate
    uint32_t actual_baud;
    uart_get_baudrate(uart_stm, &actual_baud);
    ESP_LOGI("UART", "Requested: 921600, Actual: %lu", actual_baud);
}
void uart2_mirror_task(void *arg) //debugging
{
    ESP_LOGW(TAG, "UART mirror task started, mirroring UART2 to stdout");
    uint8_t data[128];
    while (1) {
        int buffered = 0;
        uart_get_buffered_data_len(uart_stm, (size_t*)&buffered);
        ESP_LOGW(TAG, "buffered bytes: %d", buffered);
        int len = uart_read_bytes(uart_stm, data, sizeof(data)-1, pdMS_TO_TICKS(100));
        if (len > 0) {
            data[len] = 0;
            ESP_LOGW("UART", "%s", (char*)data);
        }
    }
 
    free(data); // unreachable, kept for tidiness
}
void write_data_to_stm (telemetry_data_t data) {
    telemetry_packet_t packet;
    packet.header[0] = MAGIC_BYTE_1;
    packet.header[1] = MAGIC_BYTE_2;
    packet.data_length = sizeof(data);
    memcpy(&packet.data, &data, sizeof(data));

    packet.crc = crc16((uint8_t*)&packet, offsetof(telemetry_packet_t, crc));

    uart_write_bytes(uart_stm, (const char*)&packet, sizeof(packet));
}

void read_telemetry_from_uart() {
    
    enum UART_STATE state = WAIT_AA;
    while (true) {
        uint8_t byte;
        switch (state) {
            case WAIT_AA:
                uart_read_bytes(uart_stm, &byte, 1, portMAX_DELAY);
                ESP_LOGI(TAG, "Read byte: %02X", byte);
                if (byte == MAGIC_BYTE_1) {
                    state = WAIT_55;
                }
            break;
                
            case WAIT_55:
                uart_read_bytes(uart_stm, &byte, 1, portMAX_DELAY);
                ESP_LOGI(TAG, "Read byte: %02X", byte); 
                if (byte == MAGIC_BYTE_2) {
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
                uint16_t data_length;
                uart_read_bytes(uart_stm, &data_length, sizeof(data_length), portMAX_DELAY);
                telemetry_data_t data;
                uart_read_bytes(uart_stm, &data, sizeof(data), portMAX_DELAY);
                uint16_t received_crc;
                uart_read_bytes(uart_stm, &received_crc, sizeof(received_crc), portMAX_DELAY);
                ESP_LOGI(TAG, "Read telemetry packet");
                //check crc
                telemetry_packet_t received_packet = {
                    .header = {MAGIC_BYTE_1, MAGIC_BYTE_2},
                    .data_length = data_length,
                    .data = data,
                    .crc = received_crc
                };
                uint16_t computed_crc = crc16((uint8_t*)&received_packet, 
                            offsetof(telemetry_packet_t, crc)); //lol this can definitely be rewritten so its easier to read
                if (received_crc != computed_crc) {
                    ESP_LOGW(TAG, "CRC mismatch: expected %04X, got %04X", received_crc, computed_crc);
                    state = WAIT_AA; // reset if crc is wrong
                    break;
                }
                // do something with the data here
                ESP_LOGI(TAG, "Received telemetry data: example1=%lu, example2=%lu, example3=%lu", data.example1, data.example2, data.example3);
                state = WAIT_AA; // reset for next packet
            break;
        }
    }
}



