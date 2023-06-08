#include "mailbox.h"
#include "uart.h"

volatile unsigned int __attribute__((aligned(16))) mbox[36] = {0};

int mailbox_config(unsigned char ch) {

  // We only need 0-27 bit of addr and 28-31 for channel
  unsigned int r = ((unsigned int)(((unsigned long)&mbox & ~0xf) | (ch & 0xf)));

  // Wait until the Mbox is not full
  while (*MAILBOX_STATUS & MAILBOX_FULL) {
  }

  // Write the register
  *MAILBOX_WRITE = r;

  while (1) {
    while (*MAILBOX_STATUS & MAILBOX_EMPTY) {
      asm volatile("nop");
    }
    if (r == *MAILBOX_READ){
	    uart_puth(mbox[28]);
	    uart_puts("\n");
      return mbox[1] == MAILBOX_RES;
    }

  }
  return 0;
}

int sys_mailbox_config(unsigned char ch, unsigned int *mailbox) {
  // We only need 0-27 bit of addr and 28-31 for channel
  unsigned int r =
      ((unsigned int)(((unsigned int)mailbox & ~0xf) | (ch & 0xf)));

  // Wait until the Mbox is not full
  while (*MAILBOX_STATUS & MAILBOX_FULL) {
  }

  // Write the register
  *MAILBOX_WRITE = r;

  while (1) {
    while (*MAILBOX_STATUS & MAILBOX_EMPTY) {
      asm volatile("nop");
    }

    if (r == *MAILBOX_READ){
	    uart_puth(mailbox[28]);
	    uart_puts("\n");
      return mailbox[1] == MAILBOX_RES;
    }
  }
  return 0;
}
