#include "serial_protocol.hpp"

#include <cstdarg>
#include <cstdio>

extern "C" {
#include "cmsis_os2.h"
}

namespace {

// Longest line serial_printf will send; longer output is cut off.
constexpr size_t kPrintfBufferSize = 256;

osMutexId_t serial_mutex = nullptr;

bool in_interrupt()
{
    return __get_IPSR() != 0U;
}

// Holds the mutex for the lifetime of the object, once the scheduler is running.
// Before that only one thread exists, so no lock is needed.
class SerialLock {
public:
    SerialLock()
        : locked_(serial_mutex != nullptr && osKernelGetState() == osKernelRunning)
    {
        if (locked_ && osMutexAcquire(serial_mutex, osWaitForever) != osOK) {
            locked_ = false;
        }
    }
    ~SerialLock()
    {
        if (locked_) {
            osMutexRelease(serial_mutex);
        }
    }
    SerialLock(const SerialLock&) = delete;
    SerialLock& operator=(const SerialLock&) = delete;

private:
    bool locked_;
};

HAL_StatusTypeDef transmit(const uint8_t* data, uint16_t len)
{
    // About 87 us per byte at 115200 baud, plus some slack.
    const uint32_t timeout_ms = 10U + len / 10U;
    return HAL_UART_Transmit(&hcom_uart[COM1], const_cast<uint8_t*>(data), len, timeout_ms);
}

} // namespace

void serial_init()
{
    if (serial_mutex == nullptr) {
        serial_mutex = osMutexNew(nullptr);
    }
}

HAL_StatusTypeDef serial_print(const uint8_t* str, uint16_t len)
{
    if (str == nullptr) {
        return HAL_ERROR;
    }
    if (len == 0) {
        return HAL_OK;
    }
    if (in_interrupt()) {
        return HAL_BUSY;
    }
    SerialLock lock;
    return transmit(str, len);
}

HAL_StatusTypeDef serial_printf(const char* format, ...)
{
    if (format == nullptr) {
        return HAL_ERROR;
    }
    if (in_interrupt()) {
        return HAL_BUSY;
    }

    // Static buffer keeps the formatting off the calling task's stack;
    // the lock makes it safe to share.
    static char buffer[kPrintfBufferSize];
    SerialLock lock;

    va_list args;
    va_start(args, format);
    const int written = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (written < 0) {
        return HAL_ERROR;
    }
    const size_t len = static_cast<size_t>(written) < sizeof(buffer)
                           ? static_cast<size_t>(written)
                           : sizeof(buffer) - 1;
    return transmit(reinterpret_cast<const uint8_t*>(buffer), static_cast<uint16_t>(len));
}

HAL_StatusTypeDef serial_print_byte(uint8_t byte)
{
    static const char hex_digits[] = "0123456789ABCDEF";
    const uint8_t buffer[6] = {
        '0', 'x',
        static_cast<uint8_t>(hex_digits[(byte >> 4) & 0x0F]),
        static_cast<uint8_t>(hex_digits[byte & 0x0F]),
        '\r', '\n',
    };
    return serial_print(buffer, sizeof(buffer));
}
