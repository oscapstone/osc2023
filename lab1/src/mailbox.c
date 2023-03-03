#include "mailbox.h"
#include "print.h"

volatile unsigned int __attribute__((aligned(16))) mailbox[8];

int mailbox_call(unsigned char channel) {
  unsigned int r = (((unsigned int)((unsigned long)&mailbox) & ~0xF) | (channel & 0xF));

  while ((*MAILBOX_STATUS) & MAILBOX_FULL);

  *MAILBOX_WRITE = r;

  while (1) {
    while ((*MAILBOX_STATUS) & MAILBOX_EMPTY);

    if (r == *MAILBOX_READ)
      return mailbox[1] == REQUEST_SUCCEED;
  }
  // should not return from here.
  return 0;
}

void get_board_revision() {
  mailbox[0] = 7 * 4;
  mailbox[1] = REQUEST_CODE;
  mailbox[2] = GET_BOARD_REVISION;
  mailbox[3] = 4;
  mailbox[4] = TAG_REQUEST_CODE;
  mailbox[5] = 0;
  mailbox[6] = END_TAG;

  if (mailbox_call(8)) {
    print("Board Revision: ");
    printhex(mailbox[5]);
    print("\n");
  }
  return; // return unsigned int;
}

void get_arm_memory() {
  mailbox[0] = 8 * 4;
  mailbox[1] = REQUEST_CODE;
  mailbox[2] = GET_ARM_MEMORY;
  mailbox[3] = 8;
  mailbox[4] = TAG_REQUEST_CODE;
  mailbox[5] = 0;
  mailbox[6] = 0;
  mailbox[7] = END_TAG;

  if (mailbox_call(8)) {
    print("Memory Base Address: ");
    printhex(mailbox[5]);
    print("\nMemory Size: ");
    printhex(mailbox[6]);
    print("\n");
  }
}
