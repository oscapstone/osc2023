#include "mailbox.h"

int mailbox_call(unsigned int *message) {
    unsigned long tmp = ((((unsigned long) message) & ~0x0F) | (0x08 & 0x0F));
    while (*MAILBOX_STATUS & MAILBOX_FULL) delay(1);

    *MAILBOX_WRITE = tmp;

    while (1) {
        while (*MAILBOX_STATUS & MAILBOX_EMPTY) delay(1);
        if (*MAILBOX_READ == tmp) {
            return message[1];
        }
    }
}

void get_board_revision(){
  unsigned int mailbox[7];

  mailbox[0] = 7 * 4;                // Buffer size in bytes
  mailbox[1] = REQUEST_CODE;  
  // Tags begin       
  mailbox[2] = GET_BOARD_REVISION;  // Tag identifier
  mailbox[3] = 4;                   // Maximum of request and response value buffer's length.
  mailbox[4] = TAG_REQUEST_CODE;
  mailbox[5] = 0;                   // Value buffer
  mailbox[6] = END_TAG;             
  // Tags end

  if (mailbox_call(mailbox) == REQUEST_SUCCEED) { // Message passing procedure call, you should implement it following the 6 steps provided above.
    uart_writes("Board revision:\t");
    print_hex(mailbox[5]);          // It should be 0xA020D3 for rpi3 b+
    uart_writes("\r\n");
    }    
}

void get_arm_memory() {
    unsigned int mailbox[8];

    mailbox[0] = 8 * 4;
    mailbox[1] = REQUEST_CODE;
    mailbox[2] = GET_ARM_MEMORY;
    mailbox[3] = 8;
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0;
    mailbox[6] = 0;
    mailbox[7] = END_TAG;

    if (mailbox_call(mailbox) == REQUEST_SUCCEED) {
        uart_writes("Arm memory base address:\t");
        print_hex(mailbox[5]);
        uart_writes("\r\n");
        uart_writes("Arm memory size:\t\t");
        print_hex(mailbox[6]);
        uart_writes("\r\n");
    }
}