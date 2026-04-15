#include "stm32h7xx_hal.h"

#include "stm32h7xx_nucleo.h"
#include "drivers.hpp"

extern

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
  if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
  {
    /* Retreive Rx messages from RX FIFO0 */
    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &odriveS1Handle->odriveCanRxHeader, odriveS1Handle->odriveRxBuffer) != HAL_OK)
    {
    	Error_Handler();
    }
    else {
    	odriveS1Handle->responseCallback(odriveS1Handle->odriveCanRxHeader.Identifier);
    }

    if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
    {
      Error_Handler();
    }
  }
}
