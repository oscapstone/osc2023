#include "header/mailbox.h"

volatile unsigned int  __attribute__((aligned(16))) mailbox[36];

int mailbox_call() {
    unsigned int r = (unsigned int)(((unsigned long)&mailbox) & (~0xF)) | (0x8 & 0xF);
    // wait until full flag unset
    while (*MBOX_STATUS & MBOX_FULL) {
    }
    // write address of message + channel to mailbox
    *MBOX_WRITE = r;
    // wait until response
    while (1) {
        // wait until empty flag unset
        while (*MBOX_STATUS & MBOX_EMPTY) {
        }
        // is it a response to our msg?
        if (r == *MBOX_READ) {
            // check is response success
            return mailbox[1] == MBOX_RESPONSE;
        }
    }
    return 0;
}



int get_board_revision(){
    mailbox[0] = 7 * 4; // buffer size in bytes
    mailbox[1] = MBOX_REQUEST;    
    // tags begin
    mailbox[2] = MBOX_TAG_GETBOARD; // tag identifier
    mailbox[3] = 4;          // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    // tags end
    mailbox[6] = MBOX_TAG_LAST;
    return mailbox_call(MBOX_CH_PROP); // message passing procedure call, we should implement it following the 6 steps provided above.
}


int get_arm_mem(){
    mailbox[0] = 8 * 4; // buffer size in bytes
    mailbox[1] = MBOX_REQUEST;    
    // tags begin
    mailbox[2] = MBOX_TAG_GETARMMEM; // tag identifier
    mailbox[3] = 8;          // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    mailbox[6] = 0; // value buffer
    // tags end
    mailbox[7] = MBOX_TAG_LAST;
    return mailbox_call(MBOX_CH_PROP); // message passing procedure call, we should implement it following the 6 steps provided above.
}
