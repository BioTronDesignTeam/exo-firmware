#include "safety.hpp"
#include "drivers.hpp"
#include "task_priorities.hpp"
#include "uart.hpp"

namespace {

constexpr uint32_t kSupervisorPeriodMs = 20;
constexpr uint32_t kWatchdogTimeoutMs = 500;

constexpr uint32_t kIwdgKeyStart = 0xCCCC;
constexpr uint32_t kIwdgKeyUnlock = 0x5555;
constexpr uint32_t kIwdgKeyRefresh = 0xAAAA;
constexpr uint32_t kIwdgPrescalerDiv32 = 3;

constexpr size_t kTaskCount = static_cast<size_t>(SafetyTask::Count);

struct Monitor {
    volatile uint32_t lastCheckinMs;
    uint32_t deadlineMs;
    volatile bool registered;
};

Monitor monitors[kTaskCount] = {};
volatile bool tripped = false;

void watchdog_start()
{
    DBGMCU->APB4FZ1 |= DBGMCU_APB4FZ1_DBG_IWDG1;
    IWDG1->KR = kIwdgKeyStart;
    IWDG1->KR = kIwdgKeyUnlock;
    IWDG1->PR = kIwdgPrescalerDiv32;
    IWDG1->RLR = kWatchdogTimeoutMs - 1;
    while (IWDG1->SR != 0U) {
    }
    IWDG1->KR = kIwdgKeyRefresh;
}

bool any_task_stalled(uint32_t now)
{
    for (const Monitor& monitor : monitors) {
        if (monitor.registered && now - monitor.lastCheckinMs > monitor.deadlineMs) {
            return true;
        }
    }
    return false;
}

void supervisor_task(void*)
{
    watchdog_start();
    uint32_t wake = osKernelGetTickCount();

    for (;;) {
        if (!any_task_stalled(HAL_GetTick())) {
            IWDG1->KR = kIwdgKeyRefresh;
        } else if (!tripped) {
            tripped = true;
            ODRIVES1::estopAll();
            SERIAL_PRINT("SAFETY: task stalled, motors stopped, resetting\r\n");
        }

        wake += kSupervisorPeriodMs;
        osDelayUntil(wake);
    }
}

} // namespace

bool safety_init()
{
    static const osThreadAttr_t attributes = {
        .name = "Supervisor",
        .stack_size = 1024,
        .priority = TaskPriority::Supervisor,
    };
    return osThreadNew(supervisor_task, nullptr, &attributes) != nullptr;
}

void safety_register(SafetyTask task, uint32_t deadlineMs)
{
    Monitor& monitor = monitors[static_cast<size_t>(task)];
    monitor.deadlineMs = deadlineMs;
    monitor.lastCheckinMs = HAL_GetTick();
    monitor.registered = true;
}

void safety_checkin(SafetyTask task)
{
    monitors[static_cast<size_t>(task)].lastCheckinMs = HAL_GetTick();
}

bool safety_tripped()
{
    return tripped;
}
