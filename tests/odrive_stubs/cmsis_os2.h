#pragma once
#include <cstdint>
using osThreadId_t = void*;
using osThreadFunc_t = void (*)(void*);
constexpr int osPriorityNormal = 24;
struct osThreadAttr_t { const char* name; uint32_t stack_size; int priority; };
osThreadId_t osThreadNew(osThreadFunc_t, void*, const osThreadAttr_t*);
void osDelay(uint32_t);
void osThreadExit();
