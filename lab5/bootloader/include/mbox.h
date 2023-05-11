#ifndef MBOX_H

#define MBOX_H
#include "base.h"
#define MAILBOX_BASE		(BASE+0x0000B880)
#define MAILBOX_READ		(unsigned int*)(MAILBOX_BASE+0x00000000)
#define MAILBOX_STATUS		(unsigned int*)(MAILBOX_BASE+0x00000018)
#define MAILBOX_WRITE		(unsigned int*)(MAILBOX_BASE+0x00000020)
#define MAILBOX_EMPTY		0x40000000
#define MAILBOX_FULL		0x80000000

#define GET_BOARD_REVISION	0x00010002
#define GET_ARM_MEMORY		0x00010005
#define SET_CLKRATE			0x00038002

#define REQUEST_CODE		0x00000000
#define REQUEST_SUCCEED		0x80000000
#define REQUEST_FAILED		0x80000001
#define TAG_REQUEST_CODE	0x00000000
#define END_TAG				0x00000000

#endif

int mbox_call(unsigned int* mbox,unsigned char ch);
void get_board_revision();
void get_mem_info();
