#include "mailbox.h"
#include "uart.h"

volatile unsigned int __attribute__((aligned(16))) mbox[36] = {0};


int mailbox_config(unsigned char ch){

	// We only need 0-27 bit of addr and 28-31 for channel
	unsigned int r  = ((unsigned int)(((unsigned long)&mbox & ~0xf) | (ch & 0xf)));
	
	// Wait until the Mbox is not full
	while(*MAILBOX_STATUS & MAILBOX_FULL){
	}

	// Write the register
	*MAILBOX_WRITE = r;

	while(1){
		while(*MAILBOX_STATUS & MAILBOX_EMPTY){
			asm volatile("nop");
		}

		if(r == *MAILBOX_READ)
			return mbox[1] == MAILBOX_RES;
	}
	return 0;
}



