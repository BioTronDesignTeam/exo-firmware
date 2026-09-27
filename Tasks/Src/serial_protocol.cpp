#include "serial_protocol.hpp"

#include <cstring>

extern "C" {
#include "cmsis_os2.h"
#include "main.h"
}

namespace {

constexpr uint8_t kMagicFirst = 0xAA;
constexpr uint8_t kMagicSecond = 0x55;
constexpr uint16_t kFrameOverhead = 7;
constexpr uint8_t kTxQueueDepth = 8;

struct serial_tx_message_t {
    SerialPacketType type;
    uint16_t length;
    uint8_t payload[SERIAL_PROTOCOL_MAX_PAYLOAD];
};

enum class ParserState : uint8_t {
    WaitMagicFirst,
    WaitMagicSecond,
    ReadType,
    ReadLengthLow,
    ReadLengthHigh,
    ReadPayload,
    ReadCrcLow,
    ReadCrcHigh,
};

osMessageQueueId_t serial_tx_queue = nullptr;

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

bool transmit_frame(const serial_tx_message_t& message)
{
    uint8_t frame[kFrameOverhead + SERIAL_PROTOCOL_MAX_PAYLOAD];
    frame[0] = kMagicFirst;
    frame[1] = kMagicSecond;
    frame[2] = static_cast<uint8_t>(message.type);
    frame[3] = static_cast<uint8_t>(message.length);
    frame[4] = static_cast<uint8_t>(message.length >> 8);
    std::memcpy(&frame[5], message.payload, message.length);

    const uint16_t crc = crc16_ccitt_false(&frame[2], 3 + message.length);
    frame[5 + message.length] = static_cast<uint8_t>(crc);
    frame[6 + message.length] = static_cast<uint8_t>(crc >> 8);

    return HAL_UART_Transmit(&hcom_uart[COM1], frame, kFrameOverhead + message.length, 100) == HAL_OK;
}

class SerialParser {
public:
    void consume(uint8_t byte)
    {
        switch (state_) {
        case ParserState::WaitMagicFirst:
            if (byte == kMagicFirst) {
                state_ = ParserState::WaitMagicSecond;
            }
            break;
        case ParserState::WaitMagicSecond:
            state_ = byte == kMagicSecond ? ParserState::ReadType
                                           : (byte == kMagicFirst ? ParserState::WaitMagicSecond
                                                                  : ParserState::WaitMagicFirst);
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
            if (length_ > SERIAL_PROTOCOL_MAX_PAYLOAD) {
                reset();
            } else if (length_ == 0) {
                state_ = ParserState::ReadCrcLow;
            } else {
                payload_index_ = 0;
                state_ = ParserState::ReadPayload;
            }
            break;
        case ParserState::ReadPayload:
            payload_[payload_index_++] = byte;
            if (payload_index_ == length_) {
                state_ = ParserState::ReadCrcLow;
            }
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

private:
    void dispatch_if_valid()
    {
        uint8_t crc_buffer[3 + SERIAL_PROTOCOL_MAX_PAYLOAD];
        crc_buffer[0] = static_cast<uint8_t>(type_);
        crc_buffer[1] = static_cast<uint8_t>(length_);
        crc_buffer[2] = static_cast<uint8_t>(length_ >> 8);
        std::memcpy(&crc_buffer[3], payload_, length_);

        if (crc16_ccitt_false(crc_buffer, 3 + length_) != received_crc_) {
            return;
        }

        if (type_ == SerialPacketType::Ping) {
            (void)send_serial_packet(SerialPacketType::Ack, payload_, length_);
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
};

void serial_protocol_task(void*)
{
    SerialParser parser;

    for (;;) {
        bool did_work = false;
        uint8_t byte = 0;
        if (HAL_UART_Receive(&hcom_uart[COM1], &byte, 1, 2) == HAL_OK) {
            parser.consume(byte);
            did_work = true;
        }

        serial_tx_message_t message{};
        if (osMessageQueueGet(serial_tx_queue, &message, nullptr, 0) == osOK) {
            (void)transmit_frame(message);
            did_work = true;
        }

        if (!did_work) {
            osDelay(1);
        }
    }
}

} // namespace

bool serial_protocol_init()
{
    if (serial_tx_queue != nullptr) {
        return true;
    }

    serial_tx_queue = osMessageQueueNew(kTxQueueDepth, sizeof(serial_tx_message_t), nullptr);
    if (serial_tx_queue == nullptr) {
        return false;
    }
    if (HAL_UARTEx_SetRxFifoThreshold(&hcom_uart[COM1], UART_RXFIFO_THRESHOLD_1_8) != HAL_OK ||
        HAL_UARTEx_SetTxFifoThreshold(&hcom_uart[COM1], UART_TXFIFO_THRESHOLD_1_8) != HAL_OK ||
        HAL_UARTEx_EnableFifoMode(&hcom_uart[COM1]) != HAL_OK) {
        return false;
    }

    static const osThreadAttr_t attributes = {
        .name = "SerialProtocol",
        .stack_size = 2048,
        .priority = osPriorityNormal,
    };
    return osThreadNew(serial_protocol_task, nullptr, &attributes) != nullptr;
}

bool send_serial_packet(SerialPacketType type, const void* payload, uint16_t length)
{
    if (serial_tx_queue == nullptr || length > SERIAL_PROTOCOL_MAX_PAYLOAD ||
        (length > 0 && payload == nullptr)) {
        return false;
    }

    serial_tx_message_t message{};
    message.type = type;
    message.length = length;
    if (length > 0) {
        std::memcpy(message.payload, payload, length);
    }
    return osMessageQueuePut(serial_tx_queue, &message, 0, 0) == osOK;
}

bool send_serial_log(const char* message, uint16_t length)
{
    return send_serial_packet(SerialPacketType::LogMessage, message, length);
}
