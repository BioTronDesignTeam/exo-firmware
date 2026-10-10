#include "motor_controller.hpp"
#include <cassert>
#include <cstring>
#include <string>
#include <vector>

ODRIVES1* odriveS1Handle = nullptr;
osThreadFunc_t task = nullptr;
uint32_t tick = 0;
uint32_t axisState = 1;
uint32_t primask = 0;
int scenario = 0;
int starts = 0;
int positiveVelocities = 0;
uint32_t lastPositiveTick = 0;
std::vector<std::string> logs;
struct EndSimulation {};

uint32_t HAL_GetTick() { return tick; }
uint32_t __get_PRIMASK() { return primask; }
void __disable_irq() { primask = 1; }
void __set_PRIMASK(uint32_t value) { primask = value; }
void BSP_LED_On(int) {}
void BSP_LED_Off(int) {}
int BSP_PB_GetState(int) {
    // Held for 400 ms, so a level-triggered implementation would stop again.
    return (tick >= 300 && tick < 700) || (scenario == 0 && tick >= 1000 && tick < 1100);
}
bool send_serial_log(const char* text, uint16_t length) {
    logs.emplace_back(text, length);
    return true;
}
osThreadId_t osThreadNew(osThreadFunc_t function, void*, const osThreadAttr_t* attr) {
    assert(attr->stack_size >= 2048);
    task = function;
    return reinterpret_cast<void*>(1);
}
void osThreadExit() { throw EndSimulation{}; }
void osDelay(uint32_t delay) {
    tick += delay;
    if (tick >= 3000) { throw EndSimulation{}; }
    // Scenario 1 never receives a heartbeat; 3 loses it while running.
    if (scenario != 1 && !(scenario == 3 && tick >= 800) && tick % 100 == 0) {
        odriveS1Handle->odriveCanRxHeader = {1, FDCAN_STANDARD_ID, FDCAN_DATA_FRAME, 7};
        std::memset(odriveS1Handle->odriveRxBuffer, 0, 8);
        odriveS1Handle->odriveRxBuffer[4] = axisState;
        if (scenario == 5 && tick >= 800) { odriveS1Handle->odriveRxBuffer[0] = 1; }
        assert(odriveS1Handle->responseCallback(1) == HAL_OK);
    }
}
HAL_StatusTypeDef HAL_FDCAN_ConfigFilter(FDCAN_HandleTypeDef*, FDCAN_FilterTypeDef*) { return HAL_OK; }
HAL_StatusTypeDef HAL_FDCAN_Start(FDCAN_HandleTypeDef*) { return HAL_OK; }
HAL_StatusTypeDef HAL_FDCAN_ActivateNotification(FDCAN_HandleTypeDef*, uint32_t, uint32_t) { return HAL_OK; }
HAL_StatusTypeDef HAL_FDCAN_AddMessageToTxFifoQ(FDCAN_HandleTypeDef*, FDCAN_TxHeaderTypeDef* header, uint8_t* data) {
    const uint32_t command = header->Identifier & 31;
    if (header->TxFrameType == FDCAN_REMOTE_FRAME) { return HAL_OK; }
    if (command == 0x0b) {
        uint32_t control, input;
        std::memcpy(&control, data, 4);
        std::memcpy(&input, data + 4, 4);
        assert(control == 2 && input == 1);
    }
    if (command == 7) {
        uint32_t requested;
        std::memcpy(&requested, data, 4);
        if (requested == 8) {
            ++starts;
            if (scenario != 2) { axisState = 8; } // 2 refuses closed loop
        } else { axisState = 1; }
    }
    if (command == 0x0d) {
        float velocity, torque;
        std::memcpy(&velocity, data, 4);
        std::memcpy(&torque, data + 4, 4);
        assert(torque == 0.0f);
        if (velocity > 0) {
            assert(velocity == 1.0f);
            // Motion requires a received closed-loop heartbeat, not just a queued request.
            assert(odriveS1Handle->heartbeat.axisState == 8);
            if (scenario == 4) { return HAL_ERROR; }
            ++positiveVelocities;
            lastPositiveTick = tick;
        }
    }
    return HAL_OK;
}

bool logged(const char* text) {
    for (const auto& line : logs) { if (line.find(text) != std::string::npos) { return true; } }
    return false;
}

int main() {
    FDCAN_HandleTypeDef can{};
    for (scenario = 0; scenario < 6; ++scenario) {
        tick = 0;
        axisState = 1;
        starts = positiveVelocities = 0;
        lastPositiveTick = 0;
        logs.clear();
        ODRIVES1 drive(&can);
        odriveS1Handle = &drive;
        motorControllerInitTask();
        assert(task != nullptr);
        try { task(nullptr); } catch (EndSimulation&) {}
        assert(starts <= 1); // held button/reconnect never restarts
        assert(axisState == 1);
        if (scenario == 0) {
            assert(starts == 1 && positiveVelocities > 5);
            assert(lastPositiveTick < 1000);
            assert(logged("stop requested"));
        } else if (scenario == 1) {
            assert(starts == 0 && positiveVelocities == 0);
            assert(logged("NO HEARTBEAT"));
        } else if (scenario == 2) {
            assert(starts == 1 && positiveVelocities == 0);
            assert(logged("closed-loop timeout"));
        } else if (scenario == 3) {
            assert(positiveVelocities > 0 && lastPositiveTick <= 1200);
            assert(logged("heartbeat lost or axis fault"));
        } else if (scenario == 4) {
            assert(positiveVelocities == 0 && logged("velocity TX failed"));
        } else {
            assert(positiveVelocities > 0 && lastPositiveTick < 800);
            assert(logged("heartbeat lost or axis fault"));
        }
    }
}
