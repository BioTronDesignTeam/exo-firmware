#include "serial_comms.hpp"

extern "C" 
{
#include "cmsis_os2.h"
#include "main.h"
#include "stm32h7xx_hal_def.h"
}

// sections so we can break them apart later if we want:
// protocol section - handles frame construction and decoding
// firmware section - handles interaction with RTOS and HAL
//                   also contains the exposed functions

// namespace to keep helper functions private to this file
namespace 
{ 
// === Start of protocol section ===
constexpr uint16_t SERIAL_PROTOCOL_MAX_PAYLOAD = 256;

constexpr uint8_t kMagicFirst = 0xAA;
constexpr uint8_t kMagicSecond = 0x55;

constexpr uint16_t kFrameOverhead = 7;

struct serial_tx_message_t 
{
    SerialPacketType type;
    uint16_t length;
    uint8_t payload[SERIAL_PROTOCOL_MAX_PAYLOAD];
};

uint16_t crc16_ccitt_false(const uint8_t* data, size_t length)
{
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < length; ++i) {
        crc ^= static_cast<uint16_t>(data[i]) << 8;
        for (uint8_t bit = 0; bit < 8; ++bit) {
            crc = (crc & 0x8000U) ? static_cast<uint16_t>((crc << 1) ^ 0x1021U)
                                  : static_cast<uint16_t>(crc << 1);
        }
    }
    return crc;
}

size_t build_frame(const serial_tx_message_t& message, uint8_t* out)
{
    out[0] = kMagicFirst;
    out[1] = kMagicSecond;
    out[2] = static_cast<uint8_t>(message.type);
    out[3] = static_cast<uint8_t>(message.length);
    out[4] = static_cast<uint8_t>(message.length >> 8);
    if (message.length > 0) {
        std::memcpy(&out[5], message.payload, message.length);
    }
 
    const uint16_t crc = crc16_ccitt_false(&out[2], 3 + message.length);
    out[5 + message.length] = static_cast<uint8_t>(crc);
    out[6 + message.length] = static_cast<uint8_t>(crc >> 8);
 
    return kFrameOverhead + message.length;
}

class SerialParser 
{
private:
    enum class ParserState : uint8_t 
    {
        WaitMagicFirst,
        WaitMagicSecond,
        ReadType,
        ReadLengthLow,
        ReadLengthHigh,
        ReadPayload,
        ReadCrcLow,
        ReadCrcHigh,
    };
    void dispatch_if_valid() 
    { 
        uint8_t crc_buffer[3 + SERIAL_PROTOCOL_MAX_PAYLOAD];
        crc_buffer[0] = static_cast<uint8_t>(type_);
        crc_buffer[1] = static_cast<uint8_t>(length_);
        crc_buffer[2] = static_cast<uint8_t>(length_ >> 8);
        std::memcpy(&crc_buffer[3], payload_, length_);

        if (crc16_ccitt_false(crc_buffer, 3 + length_) != received_crc_) 
        {
            return;
        }

        // should probably move message handling elsewhere 
        if (type_ == SerialPacketType::Ping) 
        {
            (void)queue_serial_packet(SerialPacketType::Ack, payload_, length_);
        }
    }

    void reset()
    {
        state_ = ParserState::WaitMagicFirst;
        length_ = 0;
        payload_index_ = 0;
        received_crc_ = 0;
    }

    ParserState state_ = ParserState::WaitMagicFirst;
    SerialPacketType type_ = SerialPacketType::Ping;
    uint16_t length_ = 0;
    uint16_t payload_index_ = 0;
    uint16_t received_crc_ = 0;
    uint8_t payload_[SERIAL_PROTOCOL_MAX_PAYLOAD] = {};

public:
    void consume(uint8_t byte)
    {
        switch (state_) 
        {
        case ParserState::WaitMagicFirst:
            if (byte == kMagicFirst) state_ = ParserState::WaitMagicSecond;
            break;
        case ParserState::WaitMagicSecond:
            if (byte == kMagicSecond) 
                state_ = ParserState::ReadType;
            else if (byte == kMagicFirst)
                state_ = ParserState::WaitMagicSecond;
            else
                state_ = ParserState::WaitMagicFirst;
            break;
        case ParserState::ReadType:
            type_ = static_cast<SerialPacketType>(byte);
            state_ = ParserState::ReadLengthLow;
            break;
        case ParserState::ReadLengthLow:
            length_ = byte;
            state_ = ParserState::ReadLengthHigh;
            break;
        case ParserState::ReadLengthHigh:
            length_ |= static_cast<uint16_t>(byte) << 8;
            if (length_ > SERIAL_PROTOCOL_MAX_PAYLOAD) 
                reset();
            else if (length_ == 0) 
                state_ = ParserState::ReadCrcLow;
            else 
            {
                payload_index_ = 0;
                state_ = ParserState::ReadPayload;
            }
            break;
        case ParserState::ReadPayload:
            payload_[payload_index_++] = byte;
            if (payload_index_ == length_) 
                state_ = ParserState::ReadCrcLow;
            break;
        case ParserState::ReadCrcLow:
            received_crc_ = byte;
            state_ = ParserState::ReadCrcHigh;
            break;
        case ParserState::ReadCrcHigh:
            received_crc_ |= static_cast<uint16_t>(byte) << 8;
            dispatch_if_valid();
            reset();
            break;
        }
    }


};

// === End of protocol section ===
// === Start of firmware section ===
osMessageQueueId_t serial_tx_queue = nullptr;
constexpr uint8_t kTxQueueDepth = 8;

void serial_tx_task(void*)
{
    serial_tx_message_t message;
    uint8_t frame[kFrameOverhead + SERIAL_PROTOCOL_MAX_PAYLOAD];
 
    for (;;)
    {
        if (osMessageQueueGet(serial_tx_queue, &message, nullptr, osWaitForever) == osOK)
        {
            size_t frame_length = build_frame(message, frame);
            (void)HAL_UART_Transmit(&hcom_uart[COM1], frame, static_cast<uint16_t>(frame_length), 100);
        }
    }
}

// could improve this to not be polling. we also do not check for overruns.
void serial_rx_task(void*)
{
    SerialParser parser;
    uint8_t byte;
 
    for (;;)
    {
        while (HAL_UART_Receive(&hcom_uart[COM1], &byte, 1, 0) == HAL_OK)
        {
            parser.consume(byte);
        }
 
        osDelay(1);
    }
}

bool init_failed()
{
    BSP_LED_On(LED_RED);
    return false;
}
 
} // namespace
 
// does not fail cleanly. but if this fails then like thats not good.
bool init_serial_tasks()
{
    static bool attempted = false;
    static bool succeeded = false;
    if (attempted)
    {
        return succeeded;
    }
    attempted = true;
 
    if (HAL_UARTEx_SetRxFifoThreshold(&hcom_uart[COM1], UART_RXFIFO_THRESHOLD_1_8) != HAL_OK ||
        HAL_UARTEx_SetTxFifoThreshold(&hcom_uart[COM1], UART_TXFIFO_THRESHOLD_1_8) != HAL_OK ||
        HAL_UARTEx_EnableFifoMode(&hcom_uart[COM1]) != HAL_OK)
    {
        return init_failed();
    }
 
    serial_tx_queue = osMessageQueueNew(kTxQueueDepth, sizeof(serial_tx_message_t), nullptr);
    if (serial_tx_queue == nullptr)
    {
        return init_failed();
    }
 
    static const osThreadAttr_t tx_attributes =
    {
        .name = "SerialTXThread",
        .stack_size = 2048,
        .priority = osPriorityNormal,
    };
    static const osThreadAttr_t rx_attributes =
    {
        .name = "SerialRXThread",
        .stack_size = 2048,
        .priority = osPriorityNormal,
    };
 
    if (osThreadNew(serial_tx_task, nullptr, &tx_attributes) == nullptr ||
        osThreadNew(serial_rx_task, nullptr, &rx_attributes) == nullptr)
    {
        return init_failed();
    }
 
    succeeded = true;
    return true;
}

bool queue_serial_packet(SerialPacketType type, const void* payload, uint16_t length)
{
    if (serial_tx_queue == nullptr ||
        length > SERIAL_PROTOCOL_MAX_PAYLOAD ||
        (length > 0 && payload == nullptr))
    {
        return false;
    }
 
    serial_tx_message_t message{};
    message.type = type;
    message.length = length;
    if (length > 0)
    {
        std::memcpy(message.payload, payload, length);
    }
    return osMessageQueuePut(serial_tx_queue, &message, 0, 0) == osOK;
}

// === End of firmware section ===
