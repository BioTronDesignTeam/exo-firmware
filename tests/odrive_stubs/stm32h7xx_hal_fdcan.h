#pragma once
#include "stm32h7xx_hal.h"
struct FDCAN_HandleTypeDef {};
struct FDCAN_FilterTypeDef {
    uint32_t IdType, FilterIndex, FilterType, FilterConfig, FilterID1, FilterID2, RxBufferIndex;
};
struct FDCAN_RxHeaderTypeDef { uint32_t Identifier, IdType, RxFrameType, DataLength; };
struct FDCAN_TxHeaderTypeDef {
    uint32_t Identifier, IdType, TxFrameType, DataLength, ErrorStateIndicator,
             BitRateSwitch, FDFormat, TxEventFifoControl, MessageMarker;
};
constexpr uint32_t FDCAN_STANDARD_ID = 0;
constexpr uint32_t FDCAN_EXTENDED_ID = 1;
constexpr uint32_t FDCAN_FILTER_MASK = 2;
constexpr uint32_t FDCAN_FILTER_TO_RXFIFO0 = 1;
constexpr uint32_t FDCAN_IT_RX_FIFO0_NEW_MESSAGE = 1;
constexpr uint32_t FDCAN_DATA_FRAME = 0;
constexpr uint32_t FDCAN_REMOTE_FRAME = 1;
constexpr uint32_t FDCAN_DLC_BYTES_8 = 8;
constexpr uint32_t FDCAN_ESI_ACTIVE = 0;
constexpr uint32_t FDCAN_BRS_OFF = 0;
constexpr uint32_t FDCAN_CLASSIC_CAN = 0;
constexpr uint32_t FDCAN_NO_TX_EVENTS = 0;
HAL_StatusTypeDef HAL_FDCAN_ConfigFilter(FDCAN_HandleTypeDef*, FDCAN_FilterTypeDef*);
HAL_StatusTypeDef HAL_FDCAN_Start(FDCAN_HandleTypeDef*);
HAL_StatusTypeDef HAL_FDCAN_ActivateNotification(FDCAN_HandleTypeDef*, uint32_t, uint32_t);
HAL_StatusTypeDef HAL_FDCAN_AddMessageToTxFifoQ(FDCAN_HandleTypeDef*, FDCAN_TxHeaderTypeDef*, uint8_t*);
