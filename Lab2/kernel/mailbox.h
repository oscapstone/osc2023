// #include "uart.h"

// #define VA_START 0xffff000000000000
// #define MMIO_BASE 0x3F000000
// // #define MMIO_BASE       0x3f000000
// #define MAILBOX_BASE    (volatile unsigned int*)(MMIO_BASE + 0xb880)

// #define MAILBOX_READ    (volatile unsigned int*)(MAILBOX_BASE)
// #define MAILBOX_POLL    (volatile unsigned int*)(MAILBOX_BASE + 0x10)
// #define MAILBOX_SENDER    (volatile unsigned int*)(MAILBOX_BASE + 0x14)
// #define MAILBOX_STATUS  (volatile unsigned int*)(MAILBOX_BASE + 0x18)
// #define MAILBOX_CONFIG  (volatile unsigned int*)(MAILBOX_BASE + 0x1C)
// #define MAILBOX_WRITE   (volatile unsigned int*)(MAILBOX_BASE + 0x20)


// #define MAILBOX_EMPTY   (0x40000000)
// #define MAILBOX_FULL    (0x80000000)
// #define MAILBOX_RESPONSE    (0x80000000)

// #define GET_BOARD_REVISION 0x00010002
// #define REQUEST_CODE 0x00000000
// #define REQUEST_SUCCEED 0x80000000
// #define REQUEST_FAILED 0x80000001
// #define TAG_REQUEST_CODE 0x00000000
// #define END_TAG 0x00000000

// #define GET_VC_MEMORY 0x00010006
// #define UART_ID 0x000000002

// /* channels */
// #define MAILBOX_CH_POWER 0
// #define MAILBOX_CH_FB 1
// #define MAILBOX_CH_VUART 2
// #define MAILBOX_CH_VCHIQ 3
// #define MAILBOX_CH_LEDS 4
// #define MAILBOX_CH_BTNS 5
// #define MAILBOX_CH_TOUCH 6
// #define MAILBOX_CH_COUNT 7
// #define MAILBOX_CH_PROP 8


// int mailbox_call(unsigned char channel);
// void get_board_revision();

#ifndef _MAILBOX_H
#define _MAILBOX_H 

#define MMIO_BASE       0x3f000000
#define MAILBOX_BASE    MMIO_BASE + 0xb880

#define MAILBOX_READ    (unsigned int*)(MAILBOX_BASE)
#define MAILBOX_STATUS  (unsigned int*)(MAILBOX_BASE + 0x18)
#define MAILBOX_WRITE   (unsigned int*)(MAILBOX_BASE  + 0x20)

#define MAILBOX_EMPTY   0x40000000
#define MAILBOX_FULL    0x80000000

#define GET_BOARD_REVISION  0x00010002
#define GET_ARM_MEMORY      0x00010005
#define REQUEST_CODE        0x00000000
#define REQUEST_SUCCEED     0x80000000
#define REQUEST_FAILED      0x80000001
#define TAG_REQUEST_CODE    0x00000000
#define END_TAG             0x00000000

void mailbox_call(unsigned int* mailbox);
void get_board_revision();
void get_memory_info();

#endif