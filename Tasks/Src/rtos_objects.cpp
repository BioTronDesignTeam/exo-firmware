#include "rtos_objects.hpp"

QueueHandle_t esp_print_queue = nullptr;
osSemaphoreId_t bno085_interrupt_semaphore_handle = nullptr;


extern "C" void initRTOSObjects(void) {
    esp_print_queue = xQueueCreate(10, sizeof(log_message_t));
    bno085_interrupt_semaphore_handle = osSemaphoreNew(1, 0, NULL);
    configASSERT(esp_print_queue != nullptr);
    configASSERT(bno085_interrupt_semaphore_handle != nullptr);

}
extern "C" void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
	if (GPIO_Pin == GPIO_PIN_6)
}
