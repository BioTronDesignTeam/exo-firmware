#include "rtos_objects.hpp"

QueueHandle_t esp_print_queue = nullptr;

extern "C" void initRTOSObjects(void) {
    esp_print_queue = xQueueCreate(10, sizeof(log_message_t));
    configASSERT(esp_print_queue != nullptr);
}
