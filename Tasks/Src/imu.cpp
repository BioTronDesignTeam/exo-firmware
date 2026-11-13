#include "imu.hpp"
#include "drivers.hpp"
#include "uart.hpp"
#include <cmsis_os2.h>
#include <string.h>
#include <stdio.h>

extern "C" {
#include "main.h"
}
//extern BNO085 *bno085Handle[5];

void update_msa311(void *arg) {
    char buf[64];
    for ( ;; ) {
        SERIAL_PRINT("Updating MSA311\r\n");
        BSP_LED_Toggle(LED_YELLOW);

        if (MSA311Handle != nullptr) {
            if (MSA311Handle->getAccel() == HAL_OK) {
                snprintf(buf, sizeof(buf), "MSA X=%.2f Y=%.2f Z=%.2f\r\n",
                         MSA311Handle->msa311_data.x,
                         MSA311Handle->msa311_data.y,
                         MSA311Handle->msa311_data.z);
                SERIAL_PRINT(buf);
            } else {
                SERIAL_PRINT("MSA read failed\r\n");
            }
        }
        osDelay(2000);
    }
}

void send_bno085_data(uint32_t index){
    uint32_t lastDiagnosticsMs = 0;
     if (bno085Handle[index] == nullptr) {
            osDelay(100);
        }

        if (!bno085Handle[index]->isInitialized()) {
            SERIAL_PRINT("BNO085: requesting product ID\r\n");
            if (!bno085Handle[index]->begin()) {
                char buffer[128];
                snprintf(buffer, sizeof(buffer),
                         "BNO085: initialization retry failed (i2c_errors=%lu malformed=%lu last=%u ch=%u data=%02X%02X%02X%02X)\r\n",
                         static_cast<unsigned long>(bno085Handle[index]->i2cErrors),
                         static_cast<unsigned long>(bno085Handle[index]->malformedPackets),
                         bno085Handle[index]->lastPacketLength,
                         bno085Handle[index]->lastPacketChannel,
                         bno085Handle[index]->lastPayloadPrefix[0],
                         bno085Handle[index]->lastPayloadPrefix[1],
                         bno085Handle[index]->lastPayloadPrefix[2],
                         bno085Handle[index]->lastPayloadPrefix[3]);
                SERIAL_PRINT(buffer);
                osDelay(1000);
                // continue;
            }
            SERIAL_PRINT("BNO085: rotation vector enabled at 100 Hz\r\n");
        }
        

        bno085Handle[index]->receiveReports();
        
        

        const uint32_t now = HAL_GetTick();
        if ((now - lastDiagnosticsMs) >= 1000U) {
            lastDiagnosticsMs = now;
            char buffer[240];
            const bno085_rot_vector_t& rotation = bno085Handle[index]->rotationVector;
            snprintf(buffer, sizeof(buffer),
                     "BNO085 q=(%.4f,%.4f,%.4f,%.4f) status=%u acc=%.3f "
                     "packets=%lu i2c_errors=%lu malformed=%lu last=%u ch=%u data=%02X%02X%02X%02X\r\n",
                     rotation.i, rotation.j, rotation.k, rotation.real,
                     rotation.status, rotation.accuracyRadians,
                     static_cast<unsigned long>(bno085Handle[index]->packetsReceived),
                     static_cast<unsigned long>(bno085Handle[index]->i2cErrors),
                     static_cast<unsigned long>(bno085Handle[index]->malformedPackets),
                     bno085Handle[index]->lastPacketLength,
                     bno085Handle[index]->lastPacketChannel,
                     bno085Handle[index]->lastPayloadPrefix[0],
                     bno085Handle[index]->lastPayloadPrefix[1],
                     bno085Handle[index]->lastPayloadPrefix[2],
                     bno085Handle[index]->lastPayloadPrefix[3]);
            SERIAL_PRINT(buffer);
        }
}

static void update_bno085(void *arg) {
    uint32_t ulNotifiedValue;
    for (;;) {
        if(xTaskNotifyWait(0, 0xFFFFFFFF, &ulNotifiedValue, portMAX_DELAY) == pdTRUE) {
            SERIAL_PRINT("interrupt received/pending");
                  BSP_LED_On(LED_YELLOW);
            for (uint32_t i=0; i<5; i++) {
                if(ulNotifiedValue & (1u << i)) {
                    send_bno085_data(i);
                }
            }
        }
    }
}

void init_imu_tasks() {
    osThreadId_t updateMSA311Handle;

    static const osThreadAttr_t updateMSA311Attributes = {
        .name = "UpdateMSA311",
        .stack_size = 1024,
        .priority = (osPriority_t) osPriorityNormal
    };
    //updateMSA311Handle = osThreadNew(update_msa311, NULL, &updateMSA311Attributes);

    static const osThreadAttr_t updateBNO085Attributes = {
        .name = "UpdateBNO085",
        .stack_size = 2048,
        .priority = (osPriority_t) osPriorityAboveNormal
    };
    updateBNO085Handle = osThreadNew(update_bno085, NULL, &updateBNO085Attributes);
}
