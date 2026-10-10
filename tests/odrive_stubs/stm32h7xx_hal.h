#pragma once
#include <cstdint>
enum HAL_StatusTypeDef { HAL_OK, HAL_ERROR };
uint32_t HAL_GetTick();
uint32_t __get_PRIMASK();
void __disable_irq();
void __set_PRIMASK(uint32_t);
