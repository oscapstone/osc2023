#include "bcm2835/mailbox.h"
#include "bcm2835/uart.h"

unsigned int __attribute__((aligned(16))) mb_buf[36];

void mb_request(unsigned int ch) {
    while (*MAILBOX_STATUS & MBOX_FULL) {/* nop */}
    unsigned int msg = ((unsigned long)mb_buf & ~0xFU) | (ch & 0xFU);
    *MAILBOX_WRITE = msg;
    do {
        while (*MAILBOX_STATUS & MBOX_EMPTY) {/* nop */}
        if ((*MAILBOX_READ & 0xFU) == ch) {
            if (mb_buf[1] == MB_RESP_ERROR) {
                uart_puts("mailbox: error parsing request buffer");
            } else if (mb_buf[1] != MB_RESP_OK) {
                uart_puts("mailbox: undefined error");
            }
            return;
        }
    } while(1);
}
