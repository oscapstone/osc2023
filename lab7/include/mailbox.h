#include "gpio.h"

extern volatile unsigned int __attribute__((aligned(16))) mbox[36];
#ifndef MAILBOX_H
#define MAILBOX_H

#define MAILBOX_BASE (MMIO_BASE + 0xb880)

#define MAILBOX_READ (volatile unsigned int *)(MAILBOX_BASE)
#define MAILBOX_STATUS (volatile unsigned int *)(MAILBOX_BASE + 0x18)
#define MAILBOX_WRITE (volatile unsigned int *)(MAILBOX_BASE + 0x20)

#define MAILBOX_EMPTY 0x40000000
#define MAILBOX_FULL 0x80000000

#define MAILBOX_REQ 0
#define MAILBOX_RES 0x80000000

#define CHANNEL_PT 0x8

#define TAG_FW_VER 0x00000001
#define TAG_BOARD_VER 0x00010002
#define TAG_ARM_MEM 0x00010005
#define TAG_LAST 0

int mailbox_config(unsigned char);
int sys_mailbox_config(unsigned char, unsigned int *);
#endif // MAILBOX_H
