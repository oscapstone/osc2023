#include "mailbox.h"
#include "utils.h"
#include "uart.h"

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
        uart_write_string("Mailbox response doesn't match!\n");
    }
}

#define MBOX_REQUEST 0
#define MBOX_CH_PROP 8
#define MBOX_TAG_LAST 0
unsigned char *set_display(unsigned *width, unsigned *height, unsigned *pitch, unsigned *isrgb)
{
    unsigned int __attribute__((aligned(16))) mbox[36];
    unsigned char *lfb;
    mbox[0] = 35 * 4;
    mbox[1] = MBOX_REQUEST;

    mbox[2] = 0x48003; // set phy wh
    mbox[3] = 8;
    mbox[4] = 8;
    mbox[5] = 1024; // FrameBufferInfo.width
    mbox[6] = 768;  // FrameBufferInfo.height

    mbox[7] = 0x48004; // set virt wh
    mbox[8] = 8;
    mbox[9] = 8;
    mbox[10] = 1024; // FrameBufferInfo.virtual_width
    mbox[11] = 768;  // FrameBufferInfo.virtual_height

    mbox[12] = 0x48009; // set virt offset
    mbox[13] = 8;
    mbox[14] = 8;
    mbox[15] = 0; // FrameBufferInfo.x_offset
    mbox[16] = 0; // FrameBufferInfo.y.offset

    mbox[17] = 0x48005; // set depth
    mbox[18] = 4;
    mbox[19] = 4;
    mbox[20] = 32; // FrameBufferInfo.depth

    mbox[21] = 0x48006; // set pixel order
    mbox[22] = 4;
    mbox[23] = 4;
    mbox[24] = 1; // RGB, not BGR preferably

    mbox[25] = 0x40001; // get framebuffer, gets alignment on request
    mbox[26] = 8;
    mbox[27] = 8;
    mbox[28] = 4096; // FrameBufferInfo.pointer
    mbox[29] = 0;    // FrameBufferInfo.size

    mbox[30] = 0x40008; // get pitch
    mbox[31] = 4;
    mbox[32] = 4;
    mbox[33] = 0; // FrameBufferInfo.pitch

    mbox[34] = MBOX_TAG_LAST;

    // this might not return exactly what we asked for, could be
    // the closest supported resolution instead
    mailbox_call(mbox, MBOX_CH_PROP);
    if (mbox[20] == 32 && mbox[28] != 0) {
        mbox[28] &= 0x3FFFFFFF; // convert GPU address to ARM address
        *width = mbox[5];        // get actual physical width
        *height = mbox[6];       // get actual physical height
        *pitch = mbox[33];       // get number of bytes per line
        *isrgb = mbox[24];       // get the actual channel order
        lfb = (void *)((unsigned long)mbox[28]);
    } else {
        uart_write_string("Unable to set screen resolution to 1024x768x32\n");
        return NULL;
    }
    return lfb;
}