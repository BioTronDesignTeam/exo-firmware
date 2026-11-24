#include "imu.hpp"
#include "drivers.hpp"
#include "serial_protocol.hpp"
#include <cmsis_os2.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

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

// The BNO085 pulls INT low when it has a packet and holds it low until the
// host reads it, so each backlog produces exactly one falling edge. If that
// edge is missed -- it happened before this task existed, or the MCU was
// reset/reflashed while the sensor stayed powered and INT was already low --
// no further edge ever comes and the sensor waits forever. So the EXTI
// notification is only a wake-up hint: the task also wakes every kBnoPollMs
// and services every sensor. receiveReports() checks the INT level itself,
// so a sensor with nothing waiting costs one GPIO read.
static constexpr uint32_t kBnoCount = 5;
static constexpr uint32_t kBnoPollMs = 10;
static constexpr uint32_t kBnoBeginRetryMs = 1000;
static constexpr uint32_t kBnoPrintPeriodMs = 1000;

static void print_bno085_diagnostics(uint32_t index, const char* what) {
    const BNO085* imu = bno085Handle[index];
    serial_printf("BNO085[%lu]: %s (packets=%lu i2c_errors=%lu malformed=%lu last=%u ch=%u data=%02X%02X%02X%02X)\r\n",
                  static_cast<unsigned long>(index), what,
                  static_cast<unsigned long>(imu->packetsReceived),
                  static_cast<unsigned long>(imu->i2cErrors),
                  static_cast<unsigned long>(imu->malformedPackets),
                  imu->lastPacketLength,
                  imu->lastPacketChannel,
                  imu->lastPayloadPrefix[0],
                  imu->lastPayloadPrefix[1],
                  imu->lastPayloadPrefix[2],
                  imu->lastPayloadPrefix[3]);
}

static void service_bno085(uint32_t index) {
    static bool beginAttempted[kBnoCount] = {};
    static uint32_t lastBeginAttemptMs[kBnoCount] = {};
    static uint32_t lastPrintMs[kBnoCount] = {};

    BNO085* imu = bno085Handle[index];
    if (imu == nullptr) {
        return;
    }

    const uint32_t now = HAL_GetTick();

    if (!imu->isInitialized()) {
        if (beginAttempted[index] && (now - lastBeginAttemptMs[index]) < kBnoBeginRetryMs) {
            return;
        }
        beginAttempted[index] = true;
        lastBeginAttemptMs[index] = now;
        serial_printf("BNO085[%lu]: requesting product ID\r\n", static_cast<unsigned long>(index));
        if (!imu->begin()) {
            print_bno085_diagnostics(index, "init failed, retrying");
            return;
        }
        serial_printf("BNO085[%lu]: reports enabled\r\n", static_cast<unsigned long>(index));
    }

    imu->receiveReports();

    if ((now - lastPrintMs[index]) >= kBnoPrintPeriodMs) {
        lastPrintMs[index] = now;
        const bno085::Sample sample = imu->latestSample();
        const bno085::OrientationReading& q = sample.orientation;

        // Quaternion -> roll/pitch/yaw in degrees (ZYX order, sensor frame
        // as printed on the board). Easier to check by hand than i/j/k/real.
        constexpr float kRadToDeg = 57.2957795f;
        const float sinPitch = 2.0f * (q.real * q.j - q.k * q.i);
        const float roll  = atan2f(2.0f * (q.real * q.i + q.j * q.k),
                                   1.0f - 2.0f * (q.i * q.i + q.j * q.j)) * kRadToDeg;
        const float pitch = asinf(fmaxf(-1.0f, fminf(1.0f, sinPitch))) * kRadToDeg;
        const float yaw   = atan2f(2.0f * (q.real * q.k + q.i * q.j),
                                   1.0f - 2.0f * (q.j * q.j + q.k * q.k)) * kRadToDeg;

        const bno085::Vector3Reading& a = sample.acceleration;
        const bno085::Vector3Reading& m = sample.magneticField;
        const float accelMag = sqrtf(a.x * a.x + a.y * a.y + a.z * a.z);  // ~9.81 at rest
        const float fieldMag = sqrtf(m.x * m.x + m.y * m.y + m.z * m.z);  // Earth: ~25-65 uT

        serial_printf("BNO085[%lu] roll=%6.1f pitch=%6.1f yaw=%6.1f st=%u err=%.2frad"
                      " | a=(%5.2f,%5.2f,%5.2f) |a|=%.2f st=%u"
                      " | |B|=%.1fuT st=%u | skipped=%lu\r\n",
                      static_cast<unsigned long>(index),
                      roll, pitch, yaw, q.status, q.accuracyRadians,
                      a.x, a.y, a.z, accelMag, a.status,
                      fieldMag, m.status,
                      static_cast<unsigned long>(imu->skippedPackets));
    }
}

static void update_bno085(void *arg) {
    (void)arg;
    for (;;) {
        uint32_t notifiedBits = 0;
        // Wake on an INT edge, or after kBnoPollMs if none arrives.
        if (xTaskNotifyWait(0, 0xFFFFFFFFu, &notifiedBits, pdMS_TO_TICKS(kBnoPollMs)) == pdTRUE &&
            notifiedBits != 0) {
            BSP_LED_Toggle(LED_YELLOW);  // blinks only on real EXTI edges
        }
        for (uint32_t i = 0; i < kBnoCount; ++i) {
            service_bno085(i);
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
        .priority = (osPriority_t) osPriorityNormal
    };
    updateBNO085Handle = osThreadNew(update_bno085, NULL, &updateBNO085Attributes);
}
