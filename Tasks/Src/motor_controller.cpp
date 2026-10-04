#include "motor_controller.hpp"
#include "serial_protocol.hpp"
#include <cstdio>
#include <cstring>

namespace {
constexpr uint32_t AXIS_IDLE = 1;
constexpr uint32_t AXIS_CLOSED_LOOP = 8;
constexpr uint32_t VELOCITY_CONTROL = 2;
constexpr uint32_t INPUT_PASSTHROUGH = 1;
constexpr uint32_t HEARTBEAT_TIMEOUT_MS = 500;
constexpr uint32_t START_TIMEOUT_MS = 2000;
constexpr float TEST_VELOCITY = 1.0f; // turns/s, zero torque feedforward

enum class MotorState { Idle, Starting, Running, Stopping };

void logMessage(const char* text)
{
    (void)send_serial_log(text, std::strlen(text));
}

void motorControllerMainLoop(void*)
{
    while (odriveS1Handle == nullptr) {
        osDelay(10);
    }
    auto& drive = *odriveS1Handle;
    if (!drive.isInitialized()) {
        logMessage("ODrive: FDCAN initialization failed");
        osThreadExit();
        return;
    }

    // Confirm idle at boot; never start automatically after a fault or reconnect.
    MotorState state = MotorState::Stopping;
    bool buttonWasPressed = BSP_PB_GetState(BUTTON_USER) == BUTTON_PRESSED;
    uint32_t buttonTick = HAL_GetTick();
    uint32_t startTick = 0;
    uint32_t lastPoll = HAL_GetTick() - 100;
    uint32_t lastLog = HAL_GetTick() - 1000;
    logMessage("ODrive: waiting for heartbeat; USER button starts/stops 1 turn/s test");

    for (;;) {
        const uint32_t now = HAL_GetTick();
        odrive_can_heartbeat_t heartbeat{};
        uint32_t heartbeatTick = 0;
        const bool connected = drive.heartbeatSnapshot(heartbeat, heartbeatTick) &&
                               now - heartbeatTick <= HEARTBEAT_TIMEOUT_MS;
        const bool pressed = BSP_PB_GetState(BUTTON_USER) == BUTTON_PRESSED;
        const bool pressEdge = pressed && !buttonWasPressed && now - buttonTick >= 200;
        buttonWasPressed = pressed;
        if (pressEdge) {
            buttonTick = now;
            if (state == MotorState::Starting || state == MotorState::Running) {
                state = MotorState::Stopping;
                logMessage("ODrive: stop requested");
            } else if (state == MotorState::Idle && connected && heartbeat.axisError == 0) {
                // Queue in order: mode, zero setpoint, then request closed loop.
                if (drive.setControllerMode(VELOCITY_CONTROL, INPUT_PASSTHROUGH) == HAL_OK &&
                    drive.setInputVelocity(0.0f, 0.0f) == HAL_OK &&
                    drive.setAxisState(AXIS_CLOSED_LOOP) == HAL_OK) {
                    startTick = now;
                    state = MotorState::Starting;
                    logMessage("ODrive: waiting for closed-loop heartbeat");
                } else {
                    state = MotorState::Stopping;
                    logMessage("ODrive: could not queue startup commands");
                }
            } else {
                logMessage("ODrive: start blocked; check heartbeat, axis errors, and idle state");
                (void)drive.getError();
            }
        }

        if (state == MotorState::Starting || state == MotorState::Running) {
            if (!connected || heartbeat.axisError != 0) {
                state = MotorState::Stopping;
                logMessage("ODrive: heartbeat lost or axis fault; stopping");
                (void)drive.getError();
            } else if (state == MotorState::Starting) {
                if (heartbeat.axisState == AXIS_CLOSED_LOOP && heartbeatTick - startTick < 0x80000000U) {
                    state = MotorState::Running;
                    logMessage("ODrive: closed loop confirmed; running at 1 turn/s");
                } else if (now - startTick >= START_TIMEOUT_MS) {
                    state = MotorState::Stopping;
                    logMessage("ODrive: closed-loop timeout; check calibration and axis errors");
                    (void)drive.getError();
                }
            } else if (heartbeat.axisState != AXIS_CLOSED_LOOP) {
                state = MotorState::Stopping;
                logMessage("ODrive: left closed loop; stopping");
                (void)drive.getError();
            }
        }

        // Feed an enabled ODrive watchdog every 50 ms, including during startup.
        if (state == MotorState::Running || state == MotorState::Starting) {
            const float velocity = state == MotorState::Running ? TEST_VELOCITY : 0.0f;
            if (drive.setInputVelocity(velocity, 0.0f) != HAL_OK) {
                state = MotorState::Stopping;
                logMessage("ODrive: velocity TX failed; stopping");
            }
        }

        if (now - lastPoll >= 100) {
            lastPoll = now;
            // Non-motion traffic also lets recent ODrive firmware detect baudrate.
            (void)drive.getHeartbeat();
            if (state == MotorState::Stopping) {
                (void)drive.setInputVelocity(0.0f, 0.0f);
                (void)drive.setAxisState(AXIS_IDLE);
                if (connected && heartbeat.axisState == AXIS_IDLE) {
                    state = MotorState::Idle;
                }
            }
        }
        if (now - lastLog >= 1000) {
            lastLog = now;
            char message[160];
            std::snprintf(message, sizeof(message),
                          "ODrive: node=%u link=%s state=%u axis_error=0x%08lx result=%u active=0x%08lx disarm=0x%08lx",
                          unsigned(ODRIVE_CAN_NODE_ID), connected ? "OK" : "NO HEARTBEAT",
                          unsigned(heartbeat.axisState), static_cast<unsigned long>(heartbeat.axisError),
                          unsigned(heartbeat.procedureResult), static_cast<unsigned long>(drive.error.activeErrors),
                          static_cast<unsigned long>(drive.error.disarmReason));
            logMessage(message);
        }
        if (connected && heartbeat.axisError == 0) { BSP_LED_Off(LED_RED); }
        else { BSP_LED_On(LED_RED); }
        if (state == MotorState::Running) { BSP_LED_On(LED_YELLOW); }
        else { BSP_LED_Off(LED_YELLOW); }
        osDelay(50);
    }
}
} // namespace

void motorControllerInitTask()
{
    static const osThreadAttr_t attributes = {
        .name = "MotorController",
        .stack_size = 2048,
        .priority = osPriorityNormal,
    };
    if (osThreadNew(motorControllerMainLoop, nullptr, &attributes) == nullptr) {
        logMessage("ODrive: failed to create motor task");
        BSP_LED_On(LED_RED);
    }
}
