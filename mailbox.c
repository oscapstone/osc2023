#include "mailbox.h"
#include "uart.h"
#include "helper.h"

void print_board_revision()
{
    unsigned int mailbox[7];

    mailbox[0] = 7 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = GET_BOARD_REVISION; // tag identifier
    mailbox[3] = 4;                  // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    // tags end
    mailbox[6] = END_TAG;

    mailbox_call(mailbox, MAILBOX_CH_ARM_TO_VC_PROP);

    uart_puts("Board Revision: 0x");
    uart_hex(mailbox[5]);
    uart_puts("\n");
}

void print_arm_memory()
{
    unsigned int mailbox[8];

    mailbox[0] = 8 * 4; // buffer size in bytes
    mailbox[1] = REQUEST_CODE;
    // tags begin
    mailbox[2] = GET_ARM_MEMORY; // tag identifier
    mailbox[3] = 8;              // maximum of request and response value buffer's length.
    mailbox[4] = TAG_REQUEST_CODE;
    mailbox[5] = 0; // value buffer
    mailbox[6] = 0; // value buffer
    // tags end
    mailbox[7] = END_TAG;

    mailbox_call(mailbox, MAILBOX_CH_ARM_TO_VC_PROP);

    uart_puts("Memory Base: 0x");
    uart_hex(mailbox[5]);
    uart_puts("\nMemory Size: 0x");
    uart_hex(mailbox[6]);
    uart_puts("\n");
}

void mailbox_call(unsigned int *mailbox, unsigned char channel)
{
    unsigned int r = (((unsigned int)((unsigned long)mailbox) & ~0xF) | (channel & 0xF));

    // wait until can write
    do
    {
        nop();
    } while (*MAILBOX_STATUS & MAILBOX_FULL);

    *MAILBOX_WRITE = r;

    // wait until can read
    do
    {
        do
        {
            nop();
        } while (*MAILBOX_STATUS & MAILBOX_EMPTY);
    } while (r != *MAILBOX_READ); // if the response is we want
}