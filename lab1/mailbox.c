#include "mailbox.h"
#include "uart.h"

volatile unsigned int __attribute__((aligned(16))) mbox[36] = {0};


int mailbox_config(unsigned char ch){

	// We only need 0-27 bit of addr and 28-31 for channel
	unsigned int r  = ((unsigned int)(((unsigned long)&mbox & ~0xf) | (ch & 0xf)));
	
	// Wait until the Mbox is not full
	while(*MAILBOX_STATUS & MAILBOX_FULL){
	}

	*MAILBOX_WRITE = r;
	/*
	uart_puth(mbox[0]);
	uart_putc('\n');
	uart_puth(mbox[1]);
	uart_putc('\n');
	uart_puth(mbox[2]);
	uart_putc('\n');
	uart_puth(mbox[3]);
	uart_putc('\n');
	uart_puth(mbox[4]);
	uart_putc('\n');
	uart_puth(r);
	uart_putc('\n');
	uart_puth((unsigned int) &mbox);
	uart_putc('\n');
	*/
	
	// FIXME: Usually we don't need this dup write. But...
	*MAILBOX_WRITE = r;
	/*
	uart_puth((unsigned int) *MAILBOX_WRITE);
	uart_putc('\n');
	uart_puth((unsigned int) *MAILBOX_READ);
	*/

	while(1){
		while(*MAILBOX_STATUS & MAILBOX_EMPTY){
			asm volatile("nop");
		}

		if(r == *MAILBOX_READ)
			return mbox[1] == MAILBOX_RES;
	}
	return 0;
}



