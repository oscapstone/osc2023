#include "peripherals/mbox_call.h"
#include "mbox_call.h"
#include "mini_uart.h"

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