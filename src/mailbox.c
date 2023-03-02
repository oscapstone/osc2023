#include "mailbox.h"
#include "utils.h"


int mailbox_write(int channel, unsigned int message) {
    unsigned int value = (message & ~0xF) | (channel & 0xF);
    while (1) {
        // Wait until mailbox is not full
        if ((*MAILBOX_STATUS & MAILBOX_FULL) == 0) {
            break;
        }
        delay(10);
    }
    // Write message to mailbox
    *MAILBOX_WRITE = value;
    return 1;
}

unsigned int mailbox_read(int channel) {
    unsigned int message;
    // Loop until we receive something from the requested channel
    while (1) {
        // Wait until mailbox is not empty
        while (*MAILBOX_STATUS & MAILBOX_EMPTY) {
            delay(10);
        }
        // Read message from mailbox
        message = *MAILBOX_READ;
        // Check if message is for the requested channel
        if ((message & 0xF) == channel) {
            // Return message data
            return (message >> 4) & 0x0FFFFFFF;
        }
    }
    return 0;
}

void mailbox_call(unsigned int *mailbox) {
    //Combine the message address (upper 28 bits) with channel number (lower 4 bits)
    unsigned int message = ((unsigned int)mailbox & ~0xF) | (MAILBOX_CHANNEL_PROP & 0xF);

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
        /*
        if ((unsigned int)mailbox != (response & ~0xF)) {
            uart_write_string("Mailbox response doesn't match!\n\r");
        } else {
            break;
        }
        */
    }
}

void __mailbox_call(unsigned int *mailbox)
{
    unsigned int r = (((unsigned int)((unsigned long)&mailbox)&~0xF) | (MAILBOX_CHANNEL_PROP&0xF));
    /* wait until we can write to the mailbox */
    do{asm volatile("nop");}while(*MAILBOX_STATUS & MAILBOX_FULL);
    /* write the address of our message to the mailbox with channel identifier */
    *MAILBOX_WRITE = r;
    /* now wait for the response */
    while(1) {
        /* is there a response? */
        do{asm volatile("nop");}while(*MAILBOX_STATUS & MAILBOX_EMPTY);
        /* is it a response to our message? */
        if(r == *MAILBOX_READ)
            /* is it a valid successful response? */
            return;
    }
    return;
}


void _mailbox_call(unsigned int *mailbox) 
{
    unsigned int message = ((unsigned int)mailbox & ~0xf) | (MAILBOX_CHANNEL_PROP & 0xf);
    // Check that the mailbox is ready
    while ((*MAILBOX_STATUS & MAILBOX_FULL) != 0) {
        delay(10);
    }
    // Write address of message to mailbox 1
    *MAILBOX_WRITE = message;
    // Wait for mailbox 0 to receive the message
    while (1) {
        while (*MAILBOX_STATUS & MAILBOX_EMPTY) {
            delay(10);
        }
        message = *MAILBOX_READ;
        if ((message & 0xF) == 1) {
            break;
        }
    }
}
/*
void mailbox_call(unsigned int *message) {
    // Make sure the message is 16-byte aligned
    unsigned int message_addr = (unsigned int)message & ~0xF;
    // Set the bottom 4 bits of the address to zero to select mailbox 0
    unsigned int mailbox_addr = (unsigned int)MAILBOX_WRITE & ~0xF;
    // Combine the message address and mailbox address to form the message
    unsigned int message_value = message_addr | (mailbox_addr & 0xF);
    // Write the message to the mailbox
    *MAILBOX_WRITE = message_value;
    // Wait for the response
    while (1) {
        // Wait until mailbox is not empty
        while (*MAILBOX_STATUS & MAILBOX_EMPTY) {
            delay(10);
        }
        // Read response from mailbox
        unsigned int response = *MAILBOX_READ;
        // Check if response is for our message
        if ((response & 0xF) == 0 && (response >> 4) == message_addr) {
            break;
        }
    }
}

*/