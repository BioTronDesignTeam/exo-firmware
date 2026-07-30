#include "uart.hpp"
#include <cmsis_os2.h>
#include "drivers.hpp"

namespace {

void uint8_to_hex(uint8_t byte, uint8_t* output, uint16_t length)
{
    static const char hex_digits[] = "0123456789ABCDEF";
    if (length < 6) {
        return;
    }
    output[0] = '0';
    output[1] = 'x';
    output[2] = hex_digits[(byte >> 4) & 0x0F];
    output[3] = hex_digits[byte & 0x0F];
    output[4] = '\r';
    output[5] = '\n';
}

void send_telemetry_to_host(void*)
{
    for (;;) {
        serial_telemetry_payload_t telemetry = {
            .timestamp_ms = HAL_GetTick(),
            .accel_x_g = 0.0f,
            .accel_y_g = 0.0f,
            .accel_z_g = 0.0f,
        };

        if (MSA311Handle != nullptr) {
            telemetry.accel_x_g = MSA311Handle->msa311_data.x;
            telemetry.accel_y_g = MSA311Handle->msa311_data.y;
            telemetry.accel_z_g = MSA311Handle->msa311_data.z;
        }

        (void)send_serial_packet(SerialPacketType::Telemetry, &telemetry, sizeof(telemetry));
        osDelay(200);
    }
}

} // namespace

HAL_StatusTypeDef esp_print_byte(uint8_t byte)
{
    uint8_t buffer[6];
    uint8_to_hex(byte, buffer, sizeof(buffer));
    return esp_print(buffer, sizeof(buffer));
}

HAL_StatusTypeDef esp_print(uint8_t* str, uint16_t len)
{
    if (str == nullptr) {
        return HAL_ERROR;
    }
    return send_serial_log(reinterpret_cast<const char*>(str), len) ? HAL_OK : HAL_ERROR;
}

void init_uart_tasks()
{
    if (!serial_protocol_init()) {
        BSP_LED_On(LED_RED);
        return;
    }

    static const osThreadAttr_t telemetry_attributes = {
        .name = "SerialTelemetry",
        .stack_size = 1024,
        .priority = osPriorityNormal,
    };
    (void)osThreadNew(send_telemetry_to_host, nullptr, &telemetry_attributes);
}
