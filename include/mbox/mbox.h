#ifndef _M_MBOX_H
#define _M_MBOX_H
#include "peripherals/base.h"

// #define MBOX_GET_FIRMWARE_REVISION (0x00000001)

extern volatile unsigned int mbox_buf[256];

#define MBOX_CH_PROPERTY_TAG (8)

// tags for get properties
#define MBOX_GET_BOARD_MODEL (0x00010001)
#define MBOX_GET_BOARD_REVISION (0x00010002)
#define MBOX_GET_ARM_MEMORY (0x00010005)

// reg address
#define MAIL_BASE (PBASE + 0xB880)


// status
#define MAIL_FULL (0x80000000)
#define MAIL_EMPTY (0x40000000)
#define MAIL_RESPONSE (0x80000000)

// mbox register address
#define MAIL_READ (MAIL_BASE + 0x0)
#define MAIL_PEEK (MAIL_BASE + 0x10)
#define MAIL_SENDER (MAIL_BASE + 0x14)
#define MAIL_STATUS (MAIL_BASE + 0x18)
#define MAIL_CONFIG (MAIL_BASE + 0x1c)
#define MAIL_WRITE (MAIL_BASE + 0x20)

int mbox_call_func(unsigned char prop_tag);
int mbox_call(unsigned char prop_tag, unsigned int *buf);
#endif
