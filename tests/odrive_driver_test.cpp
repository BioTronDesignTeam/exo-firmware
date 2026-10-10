#include "odriveS1.hpp"
#include <cassert>
#include <cstring>

FDCAN_FilterTypeDef filter{};
FDCAN_TxHeaderTypeDef sent{};
uint8_t payload[8]{};
uint32_t tick = 123;
uint32_t primask = 0;
int failStage = 0;
HAL_StatusTypeDef txResult = HAL_OK;

uint32_t HAL_GetTick() { return tick; }
uint32_t __get_PRIMASK() { return primask; }
void __disable_irq() { primask = 1; }
void __set_PRIMASK(uint32_t value) { primask = value; }
void BSP_LED_On(int) {}
HAL_StatusTypeDef HAL_FDCAN_ConfigFilter(FDCAN_HandleTypeDef*, FDCAN_FilterTypeDef* value) {
    filter = *value;
    return failStage == 1 ? HAL_ERROR : HAL_OK;
}
HAL_StatusTypeDef HAL_FDCAN_Start(FDCAN_HandleTypeDef*) { return failStage == 2 ? HAL_ERROR : HAL_OK; }
HAL_StatusTypeDef HAL_FDCAN_ActivateNotification(FDCAN_HandleTypeDef*, uint32_t, uint32_t) {
    return failStage == 3 ? HAL_ERROR : HAL_OK;
}
HAL_StatusTypeDef HAL_FDCAN_AddMessageToTxFifoQ(FDCAN_HandleTypeDef*, FDCAN_TxHeaderTypeDef* header, uint8_t* data) {
    sent = *header;
    std::memcpy(payload, data, 8);
    return txResult;
}

int main() {
    FDCAN_HandleTypeDef can{};
    ODRIVES1 drive(&can);
    assert(drive.isInitialized());
    assert(filter.FilterID1 == (ODRIVE_CAN_NODE_ID << 5));
    assert(filter.FilterID2 == 0x7e0);
    for (uint32_t node = 0; node < 64; ++node) {
        for (uint32_t cmd = 0; cmd < 32; ++cmd) {
            const bool accepted = (((node << 5) | cmd) & filter.FilterID2) == filter.FilterID1;
            assert(accepted == (node == ODRIVE_CAN_NODE_ID));
        }
    }
    assert(drive.setControllerMode(2, 1) == HAL_OK);
    assert(sent.Identifier == ((ODRIVE_CAN_NODE_ID << 5) | 0x0b));
    assert(payload[0] == 2 && payload[4] == 1);
    assert(drive.setInputVelocity(1.0f, 0.0f) == HAL_OK);
    assert(sent.Identifier == ((ODRIVE_CAN_NODE_ID << 5) | 0x0d));
    float velocity, torque;
    std::memcpy(&velocity, payload, 4);
    std::memcpy(&torque, payload + 4, 4);
    assert(velocity == 1.0f && torque == 0.0f);
    assert(drive.getHeartbeat() == HAL_OK && sent.TxFrameType == FDCAN_REMOTE_FRAME);
    txResult = HAL_ERROR;
    assert(drive.setAxisState(8) == HAL_ERROR);
    txResult = HAL_OK;
    assert(drive.sendMsgCAN(32, false) == HAL_ERROR);

    odrive_can_heartbeat_t status{};
    uint32_t receivedTick = 0;
    assert(!drive.heartbeatSnapshot(status, receivedTick));
    drive.odriveCanRxHeader = {(ODRIVE_CAN_NODE_ID << 5) | 1, FDCAN_STANDARD_ID, FDCAN_DATA_FRAME, 7};
    drive.odriveRxBuffer[4] = 8;
    assert(drive.responseCallback(drive.odriveCanRxHeader.Identifier) == HAL_OK);
    assert(drive.heartbeatSnapshot(status, receivedTick));
    assert(status.axisState == 8 && receivedTick == 123 && primask == 0);
    primask = 1;
    drive.heartbeatSnapshot(status, receivedTick);
    assert(primask == 1);
    primask = 0;

    tick = 456;
    drive.odriveRxBuffer[4] = 1;
    assert(drive.responseCallback((((ODRIVE_CAN_NODE_ID + 1) % 63) << 5) | 1) == HAL_ERROR);
    drive.odriveCanRxHeader.DataLength = 6;
    assert(drive.responseCallback(drive.odriveCanRxHeader.Identifier) == HAL_ERROR);
    drive.odriveCanRxHeader.DataLength = 7;
    drive.odriveCanRxHeader.RxFrameType = FDCAN_REMOTE_FRAME;
    assert(drive.responseCallback(drive.odriveCanRxHeader.Identifier) == HAL_ERROR);
    drive.odriveCanRxHeader.RxFrameType = FDCAN_DATA_FRAME;
    drive.odriveCanRxHeader.IdType = FDCAN_EXTENDED_ID;
    assert(drive.responseCallback(drive.odriveCanRxHeader.Identifier) == HAL_ERROR);
    drive.heartbeatSnapshot(status, receivedTick);
    assert(status.axisState == 8 && receivedTick == 123);

    for (failStage = 1; failStage <= 3; ++failStage) {
        ODRIVES1 broken(&can);
        assert(!broken.isInitialized());
        assert(broken.setAxisState(8) == HAL_ERROR);
    }
}
