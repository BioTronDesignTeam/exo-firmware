#pragma once
#include <stdint.h>
#include <string.h>
extern "C" {

#include "main.h"
#include "stm32h7xx_hal_def.h"

}

// Plain-text debug output over the ST-LINK virtual COM port
// (USART3 / BSP COM1, 115200 8N1). No framing, no queue: when a print call
// returns, the bytes have been sent. Safe to call from any task, and before the
// scheduler starts. Calls from an interrupt are ignored (they return HAL_BUSY).

#define SERIAL_PRINT(s) serial_print((const uint8_t*)(s), (uint16_t)strlen(s))
#define SERIAL_PRINTLN(s) serial_print((const uint8_t*)(s "\r\n"), (uint16_t)strlen(s "\r\n"))

// Call once at startup, before any task prints.
void serial_init();

HAL_StatusTypeDef serial_print(const uint8_t* str, uint16_t len);
HAL_StatusTypeDef serial_printf(const char* format, ...) __attribute__((format(printf, 1, 2)));
HAL_StatusTypeDef serial_print_byte(uint8_t byte);  // prints "0xAB\r\n"
