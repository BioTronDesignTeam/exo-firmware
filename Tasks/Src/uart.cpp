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
            const msa311_accel_t accel = MSA311Handle->getAccelData();
            telemetry.accel_x_g = accel.x;
            telemetry.accel_y_g = accel.y;
            telemetry.accel_z_g = accel.z;
        }
        if (bno085Handle != nullptr && bno085Handle->isInitialized()) {
            const bno085_rot_vector_t rotation = bno085Handle->getRotationVector();
            telemetry.bno_quaternion_i = rotation.i;
            telemetry.bno_quaternion_j = rotation.j;
            telemetry.bno_quaternion_k = rotation.k;
            telemetry.bno_quaternion_real = rotation.real;
            telemetry.bno_accuracy_radians = rotation.accuracyRadians;
            telemetry.bno_status = rotation.status;
            telemetry.bno_valid = 1;
        }

        (void)send_serial_packet(SerialPacketType::Telemetry, &telemetry, sizeof(telemetry));
        osDelay(200);
    }
}

} // namespace

HAL_StatusTypeDef serial_print_byte(uint8_t byte)
{
    uint8_t buffer[6];
    uint8_to_hex(byte, buffer, sizeof(buffer));
    return serial_print(buffer, sizeof(buffer));
}

HAL_StatusTypeDef serial_print(uint8_t* str, uint16_t len)
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
