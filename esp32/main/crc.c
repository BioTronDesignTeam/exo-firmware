#include "crc.h"

uint16_t crc16(uint8_t *data, uint32_t length)
{
    uint16_t crc = 0x0000; 
    for (uint32_t i = 0; i < length; i++) {
        crc ^= (uint16_t)data[i] << 8;  // bring byte into high bits
        for (int bit = 0; bit < 8; bit++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;  // CCITT poly
            else
                crc = (crc << 1);
        }
    }
    return crc;
}