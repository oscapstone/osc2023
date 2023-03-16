#include "mailbox.h"

volatile unsigned int  __attribute__((aligned(16))) mbox[32];

int mbox_call(unsigned char ch) // Mailbox 0 define several channels, but we only use channel 8 (CPU->GPU) for communication.
{
    // 1.Combine the message address (upper 28 bits) with channel number (lower 4 bits)
    unsigned int r = (((unsigned int)((unsigned long)&mbox)&~0xF) | (ch&0xF));
    
    // 2.Check if Mailbox 0 status register’s full flag is set.
    do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_FULL);
    
    // 3.If not, then you can write to Mailbox 1 Read/Write register.
    *MBOX_WRITE = r;
    
    while(1) {
        // 4.Check if Mailbox 0 status register’s empty flag is set.
        do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_EMPTY);
        
        // 6.Check if the value is the same as you wrote in step 1.
        if(r == *MBOX_READ)
            return mbox[1]==MBOX_RESPONSE;  
    }
    return 0;
}