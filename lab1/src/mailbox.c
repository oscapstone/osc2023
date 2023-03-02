#include "peripherals/mailbox.h"
#include "mini_uart.h"
#include "mailbox.h"

/* mailbox message buffer */
volatile unsigned int  __attribute__((aligned(16))) pt[64];

int mbox_call(int channel, unsigned int value)
{
    // 1. Add channel to lower 4 bit
    value &= ~(0xF);  // clear lower 4 bit
    value |= channel;
    while(1) {
        // 2. Mailbox is not full
		if((*MAILBOX_STATUS & MAILBOX_FULL) == 0) 
			break;
	}
    // 3. Write to Register
    *MAILBOX_WRITE = value;
    while(1) {
        while(1) {
            // 4. Mailbox is not empty
            if((*MAILBOX_STATUS & MAILBOX_EMPTY) == 0) 
                break;
	    }
        // 5. Read from Register
        // 6. Check if the read value is as same as "value".
        if (value == *MAILBOX_READ)
            return pt[1] == REQUEST_SUCCEED;
    }
    return 0;
}

void mbox_get_HW_Revision(){
    pt[0] = 8 * 4;                  // buffer size
    pt[1] = REQUEST_CODE;           // buffer request/response code
    pt[2] = GET_BOARD_REVISION;     // tag identifier
    pt[3] = 4;                      // value buffer size in bytes
    pt[4] = TAG_REQUEST_CODE;       // tag request/response code
    pt[5] = 0;                      // value buffer
    pt[6] = 0;                      // value buffer
    pt[7] = END_TAG;                // end tag

    if (mbox_call(MBOX_TAGS_ARM_TO_VC, (unsigned int)((unsigned long)&pt)) ) {
        uart_send_string("Hardware Revision\t: ");
        uart_hex(pt[6]); // high 32
        uart_hex(pt[5]); // low 32
        uart_send_string("\r\n");
    }
}

void mbox_get_ARM_MEM(){
    pt[0] = 8 * 4;
    pt[1] = REQUEST_CODE;
    pt[2] = GET_ARM_MEMORY;
    pt[3] = 8;
    pt[4] = TAG_REQUEST_CODE;
    pt[5] = 0;
    pt[6] = 0;
    pt[7] = END_TAG;

    if (mbox_call(MBOX_TAGS_ARM_TO_VC, (unsigned int)((unsigned long)&pt)) ) {
        uart_send_string("ARM Memory Base Address\t: ");
        uart_hex(pt[5]);
        uart_send_string("\r\n");
        uart_send_string("ARM Memory Size\t\t: ");
        uart_hex(pt[6]);
        uart_send_string("\r\n");
    }
}