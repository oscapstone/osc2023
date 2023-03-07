#include "mailbox.h"
#include "utils.h"

void mailbox_call(unsigned int *mailbox, int channel) {
    //Combine the message address (upper 28 bits) with channel number (lower 4 bits)
    unsigned int message = ((unsigned int)mailbox & ~0xF) | (channel & 0xF);

    //Check if Mailbox 0 status register’s full flag is set.
    // Wait until mailbox is not full
    while ((*MAILBOX_STATUS & MAILBOX_FULL) != 0) {
        asm volatile("nop");
    }

    //If not, then you can write to Mailbox 1 Read/Write register.
    // Write message to mailbox
    *MAILBOX_WRITE = message;

    while (1) {
        //Check if Mailbox 0 status register’s empty flag is set.
        // Wait until mailbox is not empty
        while ((*MAILBOX_STATUS & MAILBOX_EMPTY)) {
            asm volatile("nop");
        }

        //If not, then you can read from Mailbox 0 Read/Write register.
        // Read response from mailbox
        if (*MAILBOX_READ == message) return;
        uart_write_string("Mailbox response doesn't match!\n\r");
    }
}