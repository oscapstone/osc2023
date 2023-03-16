#ifndef _MAILBOX_H
#define _MAILBOX_H
#include "mmio.h"
#define MAILBOX_BASE    MMIO_BASE + 0xb880
//for mailbox address offsets, refer to https://github.com/raspberrypi/firmware/wiki/Mailboxes
//mailbox 0
#define MAILBOX_READ    (volatile unsigned int*)(MAILBOX_BASE + 0x00)
#define MAILBOX_STATUS  (volatile unsigned int*)(MAILBOX_BASE + 0x18)
//mailbox 1
#define MAILBOX_WRITE   (volatile unsigned int*)(MAILBOX_BASE + 0x20)

#define MAILBOX_EMPTY   0x40000000
#define MAILBOX_FULL    0x80000000
#define MAILBOX_CHANNEL_PROP  8
#define RESPONSE_CODE_SUCCESS 0x80000000

extern void mailbox_call(unsigned int *mailbox, int channel);
#endif