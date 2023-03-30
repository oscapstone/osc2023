#include "mbox.h"
#include "uart.h"

int mbox_call(unsigned int * mbox, unsigned char channel){
    unsigned int r = (unsigned int)((((unsigned long)mbox)&~0xF) | (channel&0xF));
    /* wait until we can write to the mailbox */
    do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    *MBOX_WRITE = r;
    /* now wait for the response */
    while(1) {
        /* is there a response? */
        do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_EMPTY);
        /* is it a response to our message? */
        if(r == *MBOX_READ)
            /* is it a valid successful response? */
            return mbox[1]==MBOX_RESPONSE;
    }
    return 0;
    
}

void mbox_board_revision(){
    // unsigned int mbox[7];
    unsigned int __attribute__((aligned(16))) mbox[7];
    mbox[0] = 7*4; //buffer size in bytes    
    mbox[1] = MBOX_REQUEST_CODE;
    //tags begun
    mbox[2] = MBOX_TAG_GET_BOARD_REVISION; // tag identifier
    mbox[3] = 4; // maximum of request and response value buffer's length
    mbox[4] = MBOX_TAG_REQUEST_CODE;
    mbox[5] = 0; // value buffer
    // tags end
    mbox[6] = MBOX_TAG_END;

    mbox_call(mbox, 8);
    
    uart_printf("mailbox board revision: 0x");
    uart_hex(mbox[5]);
    uart_printf("\n");
}

void mbox_memory_info(){
    // unsigned int mbox[8];
    unsigned int __attribute__((aligned(16))) mbox[8];
    mbox[0] = 8*4; //buffer size in bytes
    mbox[1] = MBOX_REQUEST_CODE;
    //tags begun
    mbox[2] = MBOX_TAG_GET_ARM_MEMORY; // tag identifier
    mbox[3] = 8; // maximum of request and response value buffer's length
    mbox[4] = MBOX_TAG_REQUEST_CODE;
    mbox[5] = 0; // base address
    mbox[6] = 0; // size in bytes
    // tags end
    mbox[7] = MBOX_TAG_END;
    mbox_call(mbox, 8);

    uart_printf("hardware ARM memory base address:0x");
    uart_hex(mbox[5]);
    uart_printf("\nsize:0x");
    uart_hex(mbox[6]);
    uart_printf("\n");
}