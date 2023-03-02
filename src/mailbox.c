#include "gpio.h"
#include "mini_uart.h"
#include "mailbox.h"
#include "utils.h"

/* mailbox message buffer */
volatile unsigned int  __attribute__((aligned(16))) mbox[36];

/**
 * Make a mailbox call. Returns 0 on failure, non-zero on success
 */
int mailbox_call(unsigned char ch)
{
    // step 1
    unsigned int r = (((unsigned int)((unsigned long)&mbox)&~0xF) | (ch&0xF));

    // step 2
    /* wait until we can write to the mailbox */
    do{asm volatile("nop");}while(get32(MAILBOX_STATUS) & MAILBOX_FULL);

    // step 3
    /* write the address of our message to the mailbox with channel identifier */
    put32(MAILBOX_WRITE, r);

    /* now wait for the response */
    while(1) {
        // step 4, 5
        /* is there a response? */
        do{asm volatile("nop");}while(get32(MAILBOX_STATUS) & get32(MAILBOX_EMPTY));
        
        //step 6
        /* is it a response to our message? */
        if(r == get32(MAILBOX_READ))
            /* is it a valid successful response? */
            return mbox[1]==MAILBOX_RESPONSE;
    }
    return 0;
}

void get_board_revision(void){
    mbox[0] = 7 * 4; // buffer size in bytes
    mbox[1] = REQUEST_CODE;
    // tags begin
    mbox[2] = GET_BOARD_REVISION; // tag identifier
    mbox[3] = 4; // maximum of request and response value buffer's length.
    mbox[4] = TAG_REQUEST_CODE;
    mbox[5] = 0; // value buffer
    // tags end
    mbox[6] = END_TAG;

    mailbox_call(MBOX_CH_PROP); // message passing procedure call, you should implement it following the 6 steps provided above.

    // it should be 0xa020d3 for rpi3 b+
    uart_send_string(". Board revision: ");
    uart_hex(mbox[5]);
    uart_send_string("\n");
}

void get_arm_memory(void){
    mbox[0] = 8 * 4; // buffer size in bytes
    mbox[1] = REQUEST_CODE;
    // tags begin
    mbox[2] = GET_ARM_MEMORY; // tag identifier
    mbox[3] = 8; // maximum of request and response value buffer's length.
    mbox[4] = TAG_REQUEST_CODE;
    mbox[5] = 0; // value buffer
    mbox[6] = 0;
    // tags end
    mbox[7] = END_TAG;

    mailbox_call(MBOX_CH_PROP); // message passing procedure call, you should implement it following the 6 steps provided above.

    uart_send_string(". ARM memory base address: ");
    uart_hex(mbox[5]);
    uart_send_string("\n. ARM memory size: ");
    uart_dec(mbox[6]);
    uart_send_string(" bytes\n");
}
