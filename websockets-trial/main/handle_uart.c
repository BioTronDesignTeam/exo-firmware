#include "handle_uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "websockets.h"

static const char *TAG = "UART_SIM";

void uart_sim_task(void *arg) {
    char buffer[64];

    //generate random seed
    srand((unsigned int)time(NULL));

    while (1) {
        int value1 = rand() % 100;   // 0–99
        int value2 = rand() % 1000;  // 0–999

        snprintf(buffer, sizeof(buffer),
                 "{\"temp\": %d, \"humidity\": %d}", value1, value2);

        ESP_LOGI(TAG, "Generated: %s", buffer);

        websocket_send(buffer);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}