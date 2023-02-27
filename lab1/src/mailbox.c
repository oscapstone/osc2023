#include "peripherals/mailbox.h"
#include "mailbox.h"
#include "mini_uart.h"

int mailbox_call (volatile unsigned int *mailbox) {
    unsigned int msg = ((unsigned long)mailbox & ~0xF) | (0x8 & 0xF);
    while (*MAILBOX_STATUS & MAILBOX_FULL) ;
    *MAILBOX_WRITE = msg;
    while (1) {
        while (*MAILBOX_STATUS & MAILBOX_EMPTY) {}
        if (msg == *MAILBOX_READ) {
            return mailbox[1];
        }
    }
}

void get_board_revision ()
{
    volatile unsigned int __attribute__((aligned(16))) mailbox[7];
    mailbox[0] = 7 * 4;
    mailbox[1] = REQUEST_CODE;
    mailbox[2] = GET_BOARD_REVISION;
    mailbox[3] = 4;  // response length
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0;  // receive buffer
    mailbox[6] = END_TAG;

    if (mailbox_call(mailbox) == REQUEST_SUCCEED) {
        uart_send_string("Board Revision:\t\t");
        uart_send_hex(mailbox[5]);
        uart_send_string("\r\n");
    }
}

void get_arm_memory ()
{
    volatile unsigned int __attribute__((aligned(16))) mailbox[8];
    mailbox[0] = 8 * 4;
    mailbox[1] = REQUEST_CODE;
    mailbox[2] = GET_ARM_MEMORY;
    mailbox[3] = 8;  // response length
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0;  // receive buffer
    mailbox[6] = 0;  // receive buffer
    mailbox[7] = END_TAG;

    if (mailbox_call(mailbox) == REQUEST_SUCCEED) {
        uart_send_string("Memory Base Address:\t");
        uart_send_hex(mailbox[5]);
        uart_send_string("\r\n");
        uart_send_string("Memory Size:\t\t");
        uart_send_hex(mailbox[6]);
        uart_send_string("\r\n");
    }
}