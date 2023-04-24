#include "peripherals/mailbox.h"
#include "mailbox.h"
#include "mini_uart.h"

/*
 * Make a mailbox call. Returns 0 on success.
 */
int mailbox_call(unsigned char ch, volatile unsigned int *mbox)
{
        /*
         * Combines mailbox address (aligned 16 bytes) and channel number.
         */
        unsigned int msg =
                    ((unsigned long)mbox & ~0xF) | (ch & 0xF);

        /*
         * Waits until write is available then writes the address of our message
         * to the mailbox with channel identifier.
         */
        while (*MAILBOX_STATUS & MAILBOX_FULL) { asm volatile("nop"); }
        *MAILBOX_WRITE = msg;

        while (1) {
                /*
                 * Waits for a response and checks if it is what we need.
                 */
                while (*MAILBOX_STATUS & MAILBOX_EMPTY) { asm volatile("nop"); }
                if (msg == *MAILBOX_READ) {
                        return (mbox[1] != MAILBOX_REQUEST_SUCCEED);
                }
        }
        return 1;
}

void get_board_revision(void)
{
        volatile unsigned int __attribute__((aligned(16))) mailbox[7];
        mailbox[0] = 7 * 4;                       // Length of the message
        mailbox[1] = MAILBOX_REQUEST_CODE;
        mailbox[2] = MAILBOX_GET_BOARD_REVISION;
        mailbox[3] = 4;                           // Buffer size
        mailbox[4] = MAILBOX_TAG_REQUEST_CODE;
        mailbox[5] = 0;                           // Clears output buffer
        mailbox[6] = MAILBOX_END_TAG;

        if (mailbox_call((unsigned char)MAILBOX_CH_PROP, mailbox) == 0) {
                uart_send_string("Board Revision:\t\t");
                uart_send_hex_32(mailbox[5]);
                uart_send_string("\r\n");
        }
}

void get_arm_memory(void)
{
        volatile unsigned int __attribute__((aligned(16))) mailbox[8];
        mailbox[0] = 8 * 4;                       // Length of the message
        mailbox[1] = MAILBOX_REQUEST_CODE;
        mailbox[2] = MAILBOX_GET_ARM_MEMORY;
        mailbox[3] = 8;                           // Buffer size
        mailbox[4] = MAILBOX_TAG_REQUEST_CODE;
        mailbox[5] = 0;                           // Clears output buffer
        mailbox[6] = 0;                           // Clears output buffer
        mailbox[7] = MAILBOX_END_TAG;

        if (mailbox_call((unsigned char)MAILBOX_CH_PROP, mailbox) == 0) {
                uart_send_string("Memory Base Address:\t");
                uart_send_hex_32(mailbox[5]);
                uart_send_string("\r\n");
                uart_send_string("Memory Size:\t\t");
                uart_send_hex_32(mailbox[6]);
                uart_send_string("\r\n");
        }
}