#include "driver/uart.h"
#define UART_WEBSOCKET_PORT UART_NUM_1
#define UART_BAUD           115200
#define UART_TX_PIN         17
#define UART_RX_PIN         16
#define UART_BUF_SIZE       1024   /* UART driver internal ring buffer  */

#define MAGIC_BYTE          0xAB
#define MAX_PAYLOAD_LEN     256    /* max bytes between magic and \n    */
#define CRC_LEN             2      //crc is the last 2 bytes

// full  packet: magic(1) + payload(<=256) + crc(2) 
#define MAX_FRAME_LEN       (1 + MAX_PAYLOAD_LEN + CRC_LEN)

#define RING_BUF_CAPACITY   32