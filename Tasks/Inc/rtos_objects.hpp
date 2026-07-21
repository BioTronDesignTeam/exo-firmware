#pragma once
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

#define LOG_MSG_MAX_LEN 64

typedef struct {
    uint8_t data[LOG_MSG_MAX_LEN];
    uint16_t len;
} log_message_t;

extern QueueHandle_t esp_print_queue;

#ifdef __cplusplus
extern "C" {
#endif

void initRTOSObjects();

#ifdef __cplusplus
}
#endif
