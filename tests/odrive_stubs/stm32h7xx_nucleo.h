#pragma once
constexpr int LED_RED = 0;
constexpr int LED_YELLOW = 1;
constexpr int BUTTON_USER = 0;
constexpr int BUTTON_PRESSED = 1;
void BSP_LED_On(int);
void BSP_LED_Off(int);
int BSP_PB_GetState(int);
