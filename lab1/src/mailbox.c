#include "muart.h"
#include "utils.h"
#include "mailbox.h"

int mailbox_call(unsigned int *message) {
    unsigned long msg = ((((unsigned long) message) & ~0x0F) | (0x08 & 0x0F));
    while (*MAILBOX_STATUS & MAILBOX_FULL);
    *MAILBOX_WRTIE = msg;
    while (1) {
        while (*MAILBOX_STATUS & MAILBOX_EMPTY);
        if (*MAILBOX_READ == msg) {
            return message[1];
        }
    }
}

void get_board_revision(void) {
    unsigned int mailbox[7];

    mailbox[0] = 7 * 4;                             // buffer size of bytes
    mailbox[1] = REQUEST_CODE;                      // tags begin
    mailbox[2] = GET_BOARD_REVISION;                // tag identifier
    mailbox[3] = 4;                                 // maximum of request and response value buffer's length
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0;                                 // value buffer
    mailbox[6] = END_TAG;                           // tags end

    if (mailbox_call(mailbox) == REQUEST_SUCCEED) { // message passing procedure call
        mini_uart_puts("Board Revision:\t\t");
        printhex(mailbox[5]);                       // should be 0xA020D3 for RPI3 B+
        mini_uart_puts("\n");
    }
}

void get_arm_memory(void) {
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
        mini_uart_puts("Memory Base Address:\t");
        printhex(mailbox[5]);
        mini_uart_puts("\n");
        mini_uart_puts("Memory Size:\t\t");
        printhex(mailbox[6]);
        mini_uart_puts("\n");
    }
}