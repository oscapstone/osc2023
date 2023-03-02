#include "mmio.h"

#ifndef MBOX_H
#define MBOX_H

#define MBOX_BASE (MMIO_BASE+0xb880)
// address map
#define MBOX_READ       ((volatile unsigned int*)(MBOX_BASE+0x0))
#define MBOX_STATUS     ((volatile unsigned int*)(MBOX_BASE+0x18))
#define MBOX_WRITE      ((volatile unsigned int*)(MBOX_BASE+0x20))
// flag
#define MBOX_EMPTY      0x40000000
#define MBOX_FULL       0x80000000

// tag
#define MBOX_POLL       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x10))
#define MBOX_SENDER     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x14))
#define MBOX_TAG_GET_BOARD_REVISION     0x00010002
#define MBOX_TAG_GET_ARM_MEMORY         0x00010005
#define MBOX_TAG_END   0x00000000

// other
#define MBOX_RESPONSE       0x80000000
#define MBOX_TAG_REQUEST_CODE   0x00000000
#define MBOX_REQUEST_CODE   0x00000000

#endif

int mbox_call(unsigned int*mbox, unsigned char channel);
void mbox_board_revision();
void mbox_memory_info();