#include "stm32h7xx_hal.h"

#include "stm32h7xx_nucleo.h"
#include "drivers.hpp"

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
  if (hfdcan == nullptr || hfdcan->Instance != FDCAN1)
  {
    return;
  }

  if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_MESSAGE_LOST) != RESET)
  {
    ++ODRIVES1::rxFramesLost;
  }

  if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
  {
    ODRIVES1::handleRxFifo0(hfdcan);
  }
}

void HAL_FDCAN_ErrorStatusCallback(FDCAN_HandleTypeDef *hfdcan, uint32_t ErrorStatusITs)
{
  if (hfdcan == nullptr || hfdcan->Instance != FDCAN1)
  {
    return;
  }

  if ((ErrorStatusITs & FDCAN_IT_BUS_OFF) != RESET)
  {
    ODRIVES1::recoverBusOff(hfdcan);
  }
}

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  BNO085::handleI2CEvent(hi2c, true);
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  BNO085::handleI2CEvent(hi2c, true);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
  BNO085::handleI2CEvent(hi2c, false);
}

void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c)
{
  BNO085::handleI2CEvent(hi2c, false);
}
