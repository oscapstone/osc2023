#include "mem.h"
#include "cpio.h"
#include "muart.h"
#include "shell.h"
#include "utils.h"
#include "timer.h"
#include "reboot.h"
#include "mailbox.h"
#include "exception.h"

void ls(void) {
    cpio_list();
}

void cat(void) {
    cpio_concatenate();
}

void info(void) {
    get_board_revision();
    get_arm_memory();
}

void usage(void) {
    mini_uart_puts("ls:\t\tlist the filenames in initramfs file\r\n");
    mini_uart_puts("cat:\t\tprint the contents in initramfs\r\n");
    mini_uart_puts("help:\t\tprint this help memu\r\n");
    mini_uart_puts("info:\t\tprint hardware's information\r\n");
    mini_uart_puts("hello:\t\tprint Hello World!\r\n");
    mini_uart_puts("alloc:\t\ttest for the simple_alloc function\r\n");
    mini_uart_puts("async:\t\ttest for asynchronous function\r\n");
    mini_uart_puts("reboot:\t\treboot the device\r\n");
    mini_uart_puts("execute:\trun user program from initramfs\r\n");
    mini_uart_puts("boottime:\tshow time aftering booting per two seconds\r\n");
    mini_uart_puts("multiplex:\ttest for core timer multiplexing function\r\n");
}

void alloc(void) {
    char *str = (char*) simple_alloc(8);

    for (int i = 0; i < 8; i++) {
        if (i != 7) {
            str[i] = 'A' + i;
        } else {
            str[i] = '\0';
        }
    }

    mini_uart_puts(str);
    mini_uart_puts("\r\n");
}

void hello(void) {
    mini_uart_puts("Hello World!\r\n");
}

void async(void) {
    enable_mini_uart_interrupt();

    delay(10000);

    char buffer[BUFSIZE];
    while (1) {
        async_mini_uart_puts("> ");
        unsigned int len = async_mini_uart_gets(buffer, BUFSIZE);

        if (len == 0) {
            async_mini_uart_puts("\r\n");
            continue;
        }

        async_mini_uart_puts(buffer);
        async_mini_uart_puts("\r\n");

        if (strncmp(buffer, "exit", len) == 0) {
            break;
        }
    }
    
    delay(10000);

    disable_mini_uart_interrupt();
}

void boottime(void) {
    disable_interrupt();

    core_timer_enable();
    set_core_timer(2 * get_core_frequency());

    void (*location)(void) = infinite;

    asm volatile(
        "msr     elr_el1, %0\r\n\t"
        "mov     x0, 0x340\r\n\t"
        "msr     spsr_el1, x0\r\n\t"
        "mov     x0, sp\r\n\t"
        "msr     sp_el0, x0\r\n\t"
        "eret    \r\n\t"
        ::
        "r" (location)
    );

    infinite();
}

void reboot(void) {
    mini_uart_puts("rebooting...\r\n");
    reset(100);
}

void execute(void) {
    cpio_execute();
}

void multiplex(void) {
    disable_interrupt();

    char *msg1 = (char*) simple_alloc(32);
    char *msg2 = (char*) simple_alloc(32);
    char *msg3 = (char*) simple_alloc(32);

    msg1 = "1st sent, 3rd received.\r\n";
    msg2 = "2nd sent, 2nd received.\r\n";
    msg3 = "3rd sent, 1st received.\r\n";

    add_core_timer(print_core_timer_message, msg1, 3 * get_core_frequency());
    add_core_timer(print_core_timer_message, msg2, 2 * get_core_frequency());
    add_core_timer(print_core_timer_message, msg3, 1 * get_core_frequency());

    enable_interrupt();
    core_timer_enable();

    int enable = 1;

    while (enable) {
        asm volatile("mrs %0, cntp_ctl_el0\r\n" :"=r"(enable));
    }
}

void message(char *s) {
    mini_uart_puts(s);
    mini_uart_puts(" command not found\r\n");
}