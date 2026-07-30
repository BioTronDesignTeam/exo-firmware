#pragma once
#include <stdint.h>
#include <string.h>
extern "C" {

#include "main.h"
#include "stm32h7xx_hal_def.h"

}
#include "serial_protocol.hpp"

#define ESP_PRINT(s) esp_print((uint8_t*)(s), (uint16_t)strlen(s))
#define ESP_PRINTLN(s) esp_print((uint8_t*)(s "\r\n"), (uint16_t)strlen(s "\r\n"))

HAL_StatusTypeDef esp_print_byte(uint8_t byte);
HAL_StatusTypeDef esp_print(uint8_t* str, uint16_t len);
void init_uart_tasks();


