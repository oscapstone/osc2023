#include "peripherals/mbox_call.h"
#include "mbox_call.h"
#include "mini_uart.h"
#include "stdlib.h"

void mbox_main()
{
    // get the board's unique serial number with a mailbox call
    mbox[0] = 21 * 4;       // length of the message
    mbox[1] = MBOX_REQUEST; // this is a request message

    mbox[2] = MBOX_TAG_MODEL;
    mbox[3] = 4;
    mbox[4] = 0; // ??
    mbox[5] = 0;

    mbox[6] = MBOX_TAG_REVISION;
    mbox[7] = 4;
    mbox[8] = 0; // ??
    mbox[9] = 0;

    mbox[10] = MBOX_TAG_GETSERIAL; // get serial number command
    mbox[11] = 8;                  // buffer size
    mbox[12] = 8;                  // ??
    mbox[13] = 0;                  // clear output buffer
    mbox[14] = 0;

    mbox[15] = MBOX_TAG_ARM_MEMORY; // get serial number command
    mbox[16] = 8;                   // buffer size
    mbox[17] = 8;                   // ??
    mbox[18] = 0;                   // clear output buffer
    mbox[19] = 0;

    mbox[20] = MBOX_TAG_LAST;

    // send the message to the GPU and receive answer
    if (mbox_call(MBOX_CH_PROP))
    {
        uart_send_string("Board model: ");
        uart_hex(mbox[5]);
        uart_send_string("\n");

        uart_send_string("Board revision: ");
        uart_hex(mbox[9]);
        uart_send_string("\n");

        uart_send_string("Serial number: ");
        uart_hex(mbox[14]);
        uart_hex(mbox[13]);
        uart_send_string("\n");

        uart_send_string("ARM memory base address: ");
        uart_hex(mbox[18]);
        uart_send_string("\n");

        uart_send_string("ARM memory size: ");
        uart_hex(mbox[19]);
        uart_send_string("\n");
    }
    else
    {
        uart_send_string("Unable to query serial!\n");
    }
}

// lab7, ref: https://oscapstone.github.io/labs/lab7.html
char *framebuffer_init()
{
    unsigned int __attribute__((unused)) width, height, pitch, isrgb; /* dimensions and channel order */
    char *lfb;                                                        /* raw frame buffer address */

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
    if (mbox_call(MBOX_CH_PROP) && mbox[20] == 32 && mbox[28] != 0)
    {
        mbox[28] &= 0x3FFFFFFF; // convert GPU address to ARM address
        width = mbox[5];        // get actual physical width
        height = mbox[6];       // get actual physical height
        pitch = mbox[33];       // get number of bytes per line
        isrgb = mbox[24];       // get the actual channel order
        lfb = (void *)((unsigned long)mbox[28]);
        return lfb;
    }
    else
    {
        printf("Error, framebuffer_init(), Unable to set screen resolution to 1024x768x32\r\n");
        return NULL;
    }
}