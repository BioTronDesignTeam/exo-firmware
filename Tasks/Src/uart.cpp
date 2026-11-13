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
            
            .bno085 = {},
        };

//        if (MSA311Handle != nullptr) {
//            telemetry.accel_x_g = MSA311Handle->msa311_data.x;
//            telemetry.accel_y_g = MSA311Handle->msa311_data.y;
//            telemetry.accel_z_g = MSA311Handle->msa311_data.z;
//        }
        for(int i=0; i<5; i++){
        if (bno085Handle[i] != nullptr && bno085Handle[i]->isInitialized()) {
            telemetry.bno085[i].timestamp_ms = HAL_GetTick();
            telemetry.bno085[i].accel_x_g = bno085Handle[i]->acceleration.x;
            telemetry.bno085[i].accel_y_g = bno085Handle[i]->acceleration.y;
            telemetry.bno085[i].accel_z_g = bno085Handle[i]->acceleration.z;
            telemetry.bno085[i].bno_quaternion_i = bno085Handle[i]->rotationVector.i;
            telemetry.bno085[i].bno_quaternion_j = bno085Handle[i]->rotationVector.j;
            telemetry.bno085[i].bno_quaternion_k = bno085Handle[i]->rotationVector.k;
            telemetry.bno085[i].bno_quaternion_real = bno085Handle[i]->rotationVector.real;
            telemetry.bno085[i].bno_accuracy_radians = bno085Handle[i]->rotationVector.accuracyRadians;
            telemetry.bno085[i].bno_status = bno085Handle[i]->rotationVector.status;
            telemetry.bno085[i].bno_valid = 1;
        }
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
