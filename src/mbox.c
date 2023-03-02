#include "mbox.h"
#include "uart.h"

int mbox_call(unsigned int* mbox, unsigned char channel) {
    // 1. Combine the message address (upper 28 bits) with channel number (lower 4 bits)
    unsigned int r = (unsigned int)(((unsigned long)mbox) & (~0xF)) | (channel & 0xF);
    // 2. Check if Mailbox 0 status register’s full flag is set.
    while (*MBOX_STATUS & MBOX_FULL) {
    }
    // 3. Write to Mailbox 1 Read/Write register.
    *MBOX_WRITE = r;
    // 4. Check if Mailbox 0 status register’s empty flag is set.
    while (1) {
        while (*MBOX_STATUS & MBOX_EMPTY) {
        }
        // 5. If not, then you can read from Mailbox 0 Read/Write register.
        // 6. Check if the value is the same as you wrote in step 1.
        if (r == *MBOX_READ) {
            // check is response success
            return mbox[1] == MBOX_CODE_BUF_RES_SUCC;
        }
    }
    return 0;
}

void mbox_board_revision() {
    unsigned int __attribute__((aligned(16))) mbox[7];
    mbox[0] = 7 * 4;  // buffer size in bytes
    mbox[1] = MBOX_CODE_BUF_REQ;
    // tags begin
    mbox[2] = MBOX_TAG_GET_BOARD_REVISION;  // tag identifier
    mbox[3] = 4;                            // maximum of request and response value buffer's length.
    mbox[4] = MBOX_CODE_TAG_REQ;            // tag code
    mbox[5] = 0;                            // value buffer
    mbox[6] = 0x0;                          // end tag
    // tags end
    mbox_call(mbox, 8);
    uart_printf("Board Revision: ");
    uart_2hex(mbox[5]);
    uart_printf("\n");
}

void mbox_vc_memory() {
    unsigned int __attribute__((aligned(16))) mbox[8];
    mbox[0] = 8 * 4;  // buffer size in bytes
    mbox[1] = MBOX_CODE_BUF_REQ;
    // tags begin
    mbox[2] = MBOX_TAG_GET_VC_MEMORY;  // tag identifier
    mbox[3] = 8;                       // maximum of request and response value buffer's length.
    mbox[4] = MBOX_CODE_TAG_REQ;       // tag code
    mbox[5] = 0;                       // base address
    mbox[6] = 0;                       // size in bytes
    mbox[7] = 0x0;                     // end tag
    // tags end
    mbox_call(mbox, 8);
    uart_printf("VC Core base addr: 0x%x size: 0x%x", mbox[5], mbox[6]);
    uart_2hex(mbox[5]);
    uart_printf(" size: 0x", mbox[5], mbox[6]);
    uart_2hex(mbox[6]);
    uart_printf("\n");
}