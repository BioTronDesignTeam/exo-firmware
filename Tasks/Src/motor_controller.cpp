#include "motor_controller.hpp"
#include "safety.hpp"
#include "serial_protocol.hpp"
#include "task_priorities.hpp"
#include "uart.hpp"
#include <cmath>
#include <stdio.h>

osThreadId_t motorControllerTaskHandle;

namespace {

constexpr uint32_t kLoopPeriodMs = 10;
constexpr uint32_t kCheckinDeadlineMs = 100;
constexpr uint32_t kButtonDebounceMs = 500;
constexpr uint32_t kArmTimeoutMs = 1000;
constexpr uint32_t kHeartbeatTimeoutMs = 300;
constexpr uint32_t kHostLinkTimeoutMs = 2000;
constexpr uint32_t kStatusPollMs = 50;
constexpr uint32_t kStatusLogMs = 1000;
constexpr uint32_t kMaxConsecutiveTxFailures = 5;

constexpr float kDemoVelocity = -5.0f;
constexpr float kDemoTorqueFeedForward = -0.5f;
constexpr float kMaxBusCurrent = 0.1f;
constexpr float kMaxVelocity = 6.0f;

enum class MotorState { Disarmed, Arming, Running, Fault };

const char* state_name(MotorState state)
{
    switch (state) {
        case MotorState::Disarmed: return "disarmed";
        case MotorState::Arming: return "arming";
        case MotorState::Running: return "running";
        case MotorState::Fault: return "fault";
    }
    return "?";
}

void log_message(const char* text)
{
    char buffer[96];
    snprintf(buffer, sizeof(buffer), "MOTOR: %s\r\n", text);
    SERIAL_PRINT(buffer);
}

const char* link_fault(const ODRIVES1& odrive)
{
    if (safety_tripped()) {
        return "supervisor tripped";
    }
    if (odrive.heartbeatAgeMs() > kHeartbeatTimeoutMs) {
        return "ODrive heartbeat lost";
    }
    if (serial_host_link_age_ms() > kHostLinkTimeoutMs) {
        return "host link lost";
    }
    return nullptr;
}

const char* running_fault(const ODRIVES1& odrive)
{
    const odrive_can_heartbeat_t heartbeat = odrive.read(odrive.heartbeat);
    const odrive_can_bus_t bus = odrive.read(odrive.busVoltageCurrent);
    const odrive_can_encoder_estimates_t encoder = odrive.read(odrive.encoderEstimates);

    if (heartbeat.axisError != 0U) {
        return "axis error";
    }
    if (heartbeat.axisState != static_cast<uint8_t>(AxisState::ClosedLoopControl)) {
        return "left closed loop";
    }
    if (std::fabs(bus.busCurrent) > kMaxBusCurrent) {
        return "bus over-current";
    }
    if (std::fabs(encoder.velocityEstimate) > kMaxVelocity) {
        return "over-speed";
    }
    return nullptr;
}

class MotorController {
public:
    explicit MotorController(ODRIVES1& odrive) : odrive_(odrive) {}

    void step(bool buttonPressed)
    {
        const uint32_t now = HAL_GetTick();
        const char* linkFault = link_fault(odrive_);

        switch (state_) {
            case MotorState::Disarmed:
                if (buttonPressed) {
                    if (linkFault != nullptr) {
                        log_message(linkFault);
                    } else if (odrive_.setAxisState(AxisState::ClosedLoopControl) == HAL_OK) {
                        enter(MotorState::Arming, now);
                    }
                }
                break;

            case MotorState::Arming:
                if (linkFault != nullptr) {
                    fault(linkFault, now);
                } else if (odrive_.read(odrive_.heartbeat).axisState ==
                           static_cast<uint8_t>(AxisState::ClosedLoopControl)) {
                    enter(MotorState::Running, now);
                } else if (now - stateSinceMs_ > kArmTimeoutMs) {
                    fault("arming timed out", now);
                }
                break;

            case MotorState::Running:
                if (buttonPressed) {
                    (void)odrive_.setAxisState(AxisState::Idle);
                    enter(MotorState::Disarmed, now);
                    break;
                }
                if (linkFault != nullptr) {
                    fault(linkFault, now);
                    break;
                }
                if (const char* runFault = running_fault(odrive_)) {
                    fault(runFault, now);
                    break;
                }
                sendSetpoint(now);
                break;

            case MotorState::Fault:
                if (buttonPressed && linkFault == nullptr) {
                    (void)odrive_.clearErrors(0);
                    enter(MotorState::Disarmed, now);
                }
                break;
        }

        pollStatus(now);
        updateLeds();
        logStatus(now);
    }

private:
    void enter(MotorState state, uint32_t now)
    {
        state_ = state;
        stateSinceMs_ = now;
        txFailures_ = 0;
        log_message(state_name(state));
    }

    void fault(const char* reason, uint32_t now)
    {
        (void)odrive_.estop();
        log_message(reason);
        enter(MotorState::Fault, now);
    }

    void sendSetpoint(uint32_t now)
    {
        if (odrive_.setInputVelocity(kDemoVelocity, kDemoTorqueFeedForward) == HAL_OK) {
            txFailures_ = 0;
        } else if (++txFailures_ >= kMaxConsecutiveTxFailures) {
            fault("CAN TX failing", now);
        }
    }

    void pollStatus(uint32_t now)
    {
        if (now - lastPollMs_ < kStatusPollMs) {
            return;
        }
        lastPollMs_ = now;
        (void)odrive_.getBusVoltageCurrent();
        (void)odrive_.getEncoderEstimates();
    }

    void updateLeds()
    {
        if (state_ == MotorState::Fault) {
            BSP_LED_On(LED_RED);
        } else {
            BSP_LED_Off(LED_RED);
        }
        if (state_ == MotorState::Running) {
            BSP_LED_On(LED_YELLOW);
        } else {
            BSP_LED_Off(LED_YELLOW);
        }
    }

    void logStatus(uint32_t now)
    {
        if (now - lastLogMs_ < kStatusLogMs) {
            return;
        }
        lastLogMs_ = now;
        BSP_LED_Toggle(LED_GREEN);

        const odrive_can_bus_t bus = odrive_.read(odrive_.busVoltageCurrent);
        const odrive_can_encoder_estimates_t encoder = odrive_.read(odrive_.encoderEstimates);
        char buffer[96];
        snprintf(buffer, sizeof(buffer), "MOTOR: state=%s current=%.3f velocity=%.2f tx_errors=%lu\r\n",
                 state_name(state_), bus.busCurrent, encoder.velocityEstimate,
                 static_cast<unsigned long>(odrive_.txErrors));
        SERIAL_PRINT(buffer);
    }

    ODRIVES1& odrive_;
    MotorState state_ = MotorState::Disarmed;
    uint32_t stateSinceMs_ = 0;
    uint32_t lastPollMs_ = 0;
    uint32_t lastLogMs_ = 0;
    uint32_t txFailures_ = 0;
};

void motorControllerMainLoop(void*)
{
    safety_register(SafetyTask::MotorControl, kCheckinDeadlineMs);

    while (odriveS1Handle == nullptr) {
        safety_checkin(SafetyTask::MotorControl);
        osDelay(kLoopPeriodMs);
    }

    MotorController controller(*odriveS1Handle);
    (void)odriveS1Handle->setAxisState(AxisState::Idle);

    uint32_t lastButtonMs = 0;
    uint32_t wake = osKernelGetTickCount();
    for (;;) {
        const uint32_t now = HAL_GetTick();
        bool buttonPressed = false;
        if (BSP_PB_GetState(BUTTON_USER) == BUTTON_PRESSED && now - lastButtonMs > kButtonDebounceMs) {
            lastButtonMs = now;
            buttonPressed = true;
        }

        controller.step(buttonPressed);
        safety_checkin(SafetyTask::MotorControl);

        wake += kLoopPeriodMs;
        osDelayUntil(wake);
    }
}

} // namespace

void motorControllerInitTask()
{
    static const osThreadAttr_t attributes = {
        .name = "MotorController",
        .stack_size = 2048,
        .priority = TaskPriority::MotorControl,
    };
    motorControllerTaskHandle = osThreadNew(motorControllerMainLoop, NULL, &attributes);
}
