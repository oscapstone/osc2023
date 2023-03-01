#ifndef PERIPHERALS_MAILBOX_H
#define PERIPHERALS_MAILBOX_H

#include "base.h"

#define MAILBOX_BASE    (MMIO_BASE + 0xb880)

#define MAILBOX_READ    (volatile unsigned int*) (MAILBOX_BASE)
#define MAILBOX_STATUS  (volatile unsigned int*) (MAILBOX_BASE + 0x18)
#define MAILBOX_WRITE   (volatile unsigned int*) (MAILBOX_BASE + 0x20)

#define MAILBOX_EMPTY   0x40000000
#define MAILBOX_FULL    0x80000000

#define MAILBOX_GET_BOARD_REVISION  0x00010002
#define MAILBOX_GET_ARM_MEMORY      0x00010005

#define MAILBOX_REQUEST_CODE        0x00000000
#define MAILBOX_REQUEST_SUCCEED     0x80000000
#define MAILBOX_REQUEST_FAILED      0x80000001
#define MAILBOX_TAG_REQUEST_CODE    0x00000000
#define MAILBOX_END_TAG             0x00000000

/* 
 * channels
 */
#define MAILBOX_CH_POWER   0
#define MAILBOX_CH_FB      1
#define MAILBOX_CH_VUART   2
#define MAILBOX_CH_VCHIQ   3
#define MAILBOX_CH_LEDS    4
#define MAILBOX_CH_BTNS    5
#define MAILBOX_CH_TOUCH   6
#define MAILBOX_CH_COUNT   7
#define MAILBOX_CH_PROP    8

#endif /* PERIPHERALS_MAILBOX_H */