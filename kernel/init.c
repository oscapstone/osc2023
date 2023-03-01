#include "bcm2835/mailbox.h"
#include "bcm2835/uart.h"
#include "kernel/shell.h"

static void print_hwinfo() {
    // RPI board info
    mb_buf[0] = 4 * 7;                  // mailbox buffer size
    mb_buf[1] = MB_TAG_START;
    mb_buf[2] = MB_TAG_BOARD_REVISION;
    mb_buf[3] = 4 * 1;                  // value buffer size
    mb_buf[4] &= ~(1 << 31);            // request: clear b31
    mb_buf[5] = 0;                      // value buffer
    mb_buf[6] = MB_TAG_END;
    
    mb_request(MB_CH_ARM2VC);

    if (mb_buf[1] == MB_RESP_OK) {
        uart_puts("[RPI] \n");
        uart_puts("revision:     ");
        uart_pu32h(mb_buf[5]);
        uart_send('\n');
    }
    // ARM memory info
    mb_buf[0] = 4 * 8;
    mb_buf[1] = MB_TAG_START;
    mb_buf[2] = MB_TAG_ARM_MEMORY;
    mb_buf[3] = 4 * 2;
    mb_buf[4] &= ~(1 << 31);
    mb_buf[5] = 0;          // value buffer: arm memory base address
    mb_buf[6] = 0;          // value buffer: arm memory size
    mb_buf[7] = MB_TAG_END;

    mb_request(MB_CH_ARM2VC);

    if (mb_buf[1] == MB_RESP_OK) {
        uart_puts("[ARM memory]\n");
        uart_puts("base address: ");
        uart_pu32h(mb_buf[5]);
        uart_send('\n');
        uart_puts("size:         ");
        uart_pu32h(mb_buf[6]);
        uart_send('\n');
    }
}

static void kernel_init() {
    uart_init();
}

void start_kernel() {
    kernel_init();
    print_hwinfo();
    shell_start();
}