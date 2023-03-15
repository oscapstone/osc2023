#ifndef _P_MBOX_H
#define _P_MBOX_H

#include "base.h"

#define MBASE       PBASE + 0xb880
#define M_READ      ((unsigned int*)(MBASE))
#define M_STATUS    ((unsigned int*)(MBASE + 0x18))
#define M_WRITE     ((unsigned int*)(MBASE + 0x20))

#define M_EMPTY     0x40000000
#define M_FULL      0x80000000
#define M_RESPONSE  0x80000000

#define GET_BOARD_REVISION  0x00010002
#define GET_BOARD_SERIAL    0x00010004
#define GET_MEMORY_INFO     0x00010005
#define REQUEST_CODE        0x00000000
#define REQUEST_SUCCEED     0x80000000
#define REQUEST_FAILED      0x80000001
#define TAG_REQUEST_CODE    0x00000000
#define END_TAG             0x00000000

#endif