#include "mailbox.h"
#include "mini_uart.h"


volatile unsigned int  __attribute__((aligned(16))) mbox[8];

int mbox_call(){
    unsigned int r = (((unsigned int)((unsigned long)&mbox)&~0xF) | (0x8&0xF));
    /* wait until we can write to the mailbox */
    do{
        asm volatile("nop");
    }while(*MAILBOX_STATUS & MAILBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    *MAILBOX_WRITE = r;

    /* now wait for the response */
    while(1) {
        /* is there a response? */
        do{
            asm volatile("nop");
        }while(*MAILBOX_STATUS & MAILBOX_EMPTY);

        /* is it a response to our message? */
        if(r == *MAILBOX_READ)
            /* is it a valid successful response? */
            return mbox[1]==MAILBOX_RESPONSE;
    }
    return 0;
}


void board_revision(){
    mbox[0] = 7 * 4; // buffer size in bytes
    mbox[1] = REQUEST_CODE;
    // tags begin
    mbox[2] = GET_BOARD_REVISION; // tag identifier
    mbox[3] = 4;                  // maximum of request and response value buffer's length.
    mbox[4] = TAG_REQUEST_CODE;
    mbox[5] = 0; // value buffer
    // tags end
    mbox[6] = END_TAG;
    mbox_call(); // message passing procedure call, you should implement it following the 6 steps provided above.
    uart_display("board_revision : ");
    uart_hex(mbox[5]);
    uart_display("\r\n");
}

void arm_memory()
{
    mbox[0] = 8 * 4; // buffer size in bytes
    mbox[1] = REQUEST_CODE;
    // tags begin
    mbox[2] = ARM_MEMORY; // tag identifier
    mbox[3] = 8;          // maximum of request and response value buffer's length.
    mbox[4] = TAG_REQUEST_CODE;
    mbox[5] = 0; // value buffer
    mbox[6] = 0; // value buffer
    // tags end
    mbox[7] = END_TAG;
    mbox_call(); // message passing procedure call, you should implement it following the 6 steps provided above.
    uart_display("Arm base address : ");
    uart_hex(mbox[5]);
    uart_display("\r\n");
    uart_display("Arm memory size : ");
    uart_hex(mbox[6]);
    uart_display("\r\n");
}
