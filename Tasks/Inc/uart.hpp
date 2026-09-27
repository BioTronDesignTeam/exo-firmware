#pragma once
#include <stdint.h>
#include <string.h>
extern "C" {

#include "main.h"
#include "stm32h7xx_hal_def.h"

}
#include "serial_protocol.hpp"

#define SERIAL_PRINT(s) serial_print((uint8_t*)(s), (uint16_t)strlen(s))
#define SERIAL_PRINTLN(s) serial_print((uint8_t*)(s "\r\n"), (uint16_t)strlen(s "\r\n"))

HAL_StatusTypeDef serial_print_byte(uint8_t byte);
HAL_StatusTypeDef serial_print(uint8_t* str, uint16_t len);
void init_uart_tasks();


