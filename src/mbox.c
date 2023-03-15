#include "mini_uart.h"
#include "peripherals/mbox.h"

volatile unsigned int mailbox[8];

int mailbox_call() {
    unsigned int r = ( ((unsigned int)((unsigned long)&mailbox) & ~0xF) | (8&0xF) );
    while(*(M_STATUS) & M_FULL);
    *M_WRITE = r;
    while(1) {
        while(*(M_STATUS) & M_EMPTY);
        if(r == *(M_READ))
            return mailbox[1] == M_RESPONSE;
    }
    return 0;
}

void get_board_revision(){
    mailbox[0] = 7 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = GET_BOARD_REVISION; // tag identifier
    mailbox[3] = 4; // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    // tags end
    mailbox[6] = END_TAG;
    

    mailbox_call(); // message passing procedure call, you should implement it following the 6 steps provided above.
    //uart_send_hex(mailbox[1]);
    uart_send_string("Board revision: ");
    uart_send_hex(mailbox[5]); // it should be 0xa020d3 for rpi3 b+
    uart_send_string("\r\n");
}

void get_memory_info() {
    mailbox[0] = 8 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = GET_MEMORY_INFO; // tag identifier
    mailbox[3] = 8; // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    mailbox[6] = 0; // value buffer
    // tags end
    mailbox[7] = END_TAG;
    
    mailbox_call();
    uart_send_string("Base address: ");
    uart_send_hex(mailbox[5]);
    uart_send_string("\r\n");
    uart_send_string("Memory size: ");
    uart_send_hex(mailbox[6]);
}