#include "utils.h"
#include "peripherals/mbox_call.h"
#include "peripherals/gpio.h"

/* mailbox message buffer */
volatile unsigned int __attribute__((aligned(16))) mbox[36];

/**
 * Make a mailbox call. Returns 0 on failure, non-zero on success
 */
int mbox_call(unsigned char ch)
{
    unsigned int r = (((unsigned int)((unsigned long)&mbox) & ~0xF) | (ch & 0xF));

    /* wait until we can write to the mailbox */
    while (1)
    {
        if (!(get32(MBOX_STATUS) & MBOX_FULL))
            break;
    }

    /* write the address of our message to the mailbox with channel identifier */
    put32(MBOX_WRITE, r);

    /* now wait for the response */
    while (1)
    {
        /* is there a response? */
        while (1)
        {
            if (!(get32(MBOX_STATUS) & MBOX_EMPTY))
                break;
        }

        /* is it a response to our message? */
        if (r == get32(MBOX_READ))
            /* is it a valid successful response? */
            return mbox[1] == MBOX_RESPONSE;
    }

    return 0;
}