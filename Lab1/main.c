#include "uart.h"
#include "mbox.h"

/* mbox */
void get_board_revision(){
    mbox[0] = 7 * 4; // buffer size in bytes
    mbox[1] = MBOX_REQUEST;
    // tags begin
    mbox[2] = 0x00010002; // tag identifier : GET_BOARD_REVISION
    mbox[3] = 4; // maximum of request and response value buffer's length.
    mbox[4] = 4;
    mbox[5] = 0; // value buffer
    // tags end
    mbox[6] = MBOX_TAG_LAST;

    mbox_call(MBOX_CH_PROP); // message passing procedure call, you should implement it following the 6 steps provided above.

    uart_puts("board version:    0x");
    uart_hex(mbox[5]);
    uart_puts("\n");
}

void get_arm_memory(){
    mbox[0] = 8 * 4; // buffer size in bytes
    mbox[1] = MBOX_REQUEST;
    // tags begin
    mbox[2] = 0x00010005; // tag identifier: GET_ARM_MEMORY
    mbox[3] = 8; // maximum of request and response value buffer's length.
    mbox[4] = 8;
    mbox[5] = 0; // value buffer
    mbox[6] = 0; // value buffer
    // tags end
    mbox[7] = MBOX_TAG_LAST;

    mbox_call(MBOX_CH_PROP); // message passing procedure call, you should implement it following the 6 steps provided above.

    uart_puts("Arm base address: 0x");
    uart_hex(mbox[5]);
    uart_puts("\n");
    uart_puts("Arm memory size:  0x");
    uart_hex(mbox[6]);
    uart_puts("\n");
}

/* reboot */
#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

void set(long addr, unsigned int value) {
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

void reset(int tick) {                 // reboot after watchdog timer expire
    set(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    set(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}

void cancel_reset() {
    set(PM_RSTC, PM_PASSWORD | 0);  // full reset
    set(PM_WDOG, PM_PASSWORD | 0);  // number of watchdog tick
}

/* strcmp */
int strcmp(char* str1, char* str2) {
    while(1) {
        if ((*str1 == '\0' || *str1 == '\n') && (*str2 == '\0' || *str2 == '\n')) {
            return 0;
        }
        if (*str1 != *str2) {
            return 1;
        }
        str1++;
        str2++;
    }
}

/* shell */
void main(void) {
    uart_init();
    uart_puts("! Welcome Lab1 !\n");

    char command[32];
    int idx = 0;

    while (1) {
        idx = 0;
        uart_puts("$ ");
        while (1) {
            command[idx] = uart_getc();
            uart_send(command[idx]);
            if (command[idx] == '\n') {
                command[idx] = '\0';
                break;
            }
            idx++;
        }

        if (strcmp("hello", command) == 0) {
            uart_puts("\n");
            uart_puts("Hello World!\n");
        }
        else if (strcmp("help", command) == 0) {
            uart_puts("\n");
            uart_puts("help\t: print this help menu\n");
            uart_puts("hello\t: print Hello World!\n");
            uart_puts("reboot\t: reboot this device\n");
            uart_puts("lshw\t: print hardware info from mailbox\n");
        }
        else if (strcmp("reboot", command) == 0) {
            uart_puts("\n");
            uart_puts("rebooting...\r\n");
            reset(1000);
        }
        else if (strcmp("lshw", command) == 0) {
            uart_puts("\n");
            get_board_revision();
            get_arm_memory();
        }
        else {
            uart_puts("\n");
            uart_puts("unknown\n");
        }
        
    }
        
}

