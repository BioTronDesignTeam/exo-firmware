#include "imu.hpp"
#include "drivers.hpp"
#include "uart.hpp"
#include <cmsis_os2.h>
#include "task_priorities.hpp"
#include <string.h>
#include <stdio.h>

extern "C" {
#include "main.h"
}

void update_msa311(void *arg) {
    char buf[64];
    for ( ;; ) {
        if (MSA311Handle != nullptr) {
            if (MSA311Handle->getAccel() == HAL_OK) {
                const msa311_accel_t accel = MSA311Handle->getAccelData();
                snprintf(buf, sizeof(buf), "MSA X=%.2f Y=%.2f Z=%.2f\r\n",
                         accel.x, accel.y, accel.z);
                SERIAL_PRINT(buf);
            } else {
                SERIAL_PRINT("MSA read failed\r\n");
            }
        }
        osDelay(2000);
    }
}

static void update_bno085(void *arg) {
    uint32_t lastDiagnosticsMs = 0;
    bool reportedUnavailable = false;

    for (;;) {
        if (bno085Handle == nullptr) {
            if (!reportedUnavailable) {
                SERIAL_PRINT("BNO085 initialization failed\r\n");
                reportedUnavailable = true;
            }
            osDelay(100);
            continue;
        }

        reportedUnavailable = false;
        if (!bno085Handle->isInitialized()) {
            SERIAL_PRINT("BNO085: requesting product ID\r\n");
            if (!bno085Handle->begin()) {
                char buffer[128];
                snprintf(buffer, sizeof(buffer),
                         "BNO085: initialization retry failed (i2c_errors=%lu malformed=%lu last=%u ch=%u data=%02X%02X%02X%02X)\r\n",
                         static_cast<unsigned long>(bno085Handle->i2cErrors),
                         static_cast<unsigned long>(bno085Handle->malformedPackets),
                         bno085Handle->lastPacketLength,
                         bno085Handle->lastPacketChannel,
                         bno085Handle->lastPayloadPrefix[0],
                         bno085Handle->lastPayloadPrefix[1],
                         bno085Handle->lastPayloadPrefix[2],
                         bno085Handle->lastPayloadPrefix[3]);
                SERIAL_PRINT(buffer);
                osDelay(1000);
                continue;
            }
            SERIAL_PRINT("BNO085: rotation vector enabled at 100 Hz\r\n");
        }

        const osStatus_t wakeStatus =
            osSemaphoreAcquire(bno085_interrupt_semaphore_handle, 20U);

        // INT is active-low and remains asserted while packets are queued.
        // Checking the pin after a timeout also handles a packet that was
        // already pending before this task started waiting.
        if (wakeStatus == osOK ||
            HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6) == GPIO_PIN_RESET) {
            for (uint8_t packet = 0;
                 packet < 8U &&
                 HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_6) == GPIO_PIN_RESET;
                 ++packet) {
                bno085Handle->receiveReports();
            }
        }

        const uint32_t now = HAL_GetTick();
        if ((now - lastDiagnosticsMs) >= 1000U) {
            lastDiagnosticsMs = now;
            char buffer[240];
            const bno085_rot_vector_t rotation = bno085Handle->getRotationVector();
            snprintf(buffer, sizeof(buffer),
                     "BNO085 q=(%.4f,%.4f,%.4f,%.4f) status=%u acc=%.3f "
                     "packets=%lu i2c_errors=%lu malformed=%lu last=%u ch=%u data=%02X%02X%02X%02X\r\n",
                     rotation.i, rotation.j, rotation.k, rotation.real,
                     rotation.status, rotation.accuracyRadians,
                     static_cast<unsigned long>(bno085Handle->packetsReceived),
                     static_cast<unsigned long>(bno085Handle->i2cErrors),
                     static_cast<unsigned long>(bno085Handle->malformedPackets),
                     bno085Handle->lastPacketLength,
                     bno085Handle->lastPacketChannel,
                     bno085Handle->lastPayloadPrefix[0],
                     bno085Handle->lastPayloadPrefix[1],
                     bno085Handle->lastPayloadPrefix[2],
                     bno085Handle->lastPayloadPrefix[3]);
            SERIAL_PRINT(buffer);
        }
    }
}

void init_imu_tasks() {
    static const osThreadAttr_t updateMSA311Attributes = {
        .name = "UpdateMSA311",
        .stack_size = 1024,
        .priority = TaskPriority::Diagnostics
    };
    osThreadNew(update_msa311, NULL, &updateMSA311Attributes);

    static const osThreadAttr_t updateBNO085Attributes = {
        .name = "UpdateBNO085",
        .stack_size = 2048,
        .priority = TaskPriority::Imu
    };
    osThreadNew(update_bno085, NULL, &updateBNO085Attributes);
}
