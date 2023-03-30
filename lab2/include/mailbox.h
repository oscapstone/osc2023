#ifndef _MAILBOX_H_
#define _MAILBOX_H_

extern volatile unsigned int pt[64];

int mbox_call(int , unsigned int);
void mbox_get_HW_Revision();
void mbox_get_ARM_MEM();

#endif /*_MAILBOX_H_*/