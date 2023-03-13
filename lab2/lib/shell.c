#include "cpio.h"
#include "muart.h"
#include "shell.h"
#include "reboot.h"
#include "mailbox.h"

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
    mini_uart_puts("ls:\tlist the filenames in initramfs file\r\n");
    mini_uart_puts("cat:\tprint the contents in initramfs\r\n");
    mini_uart_puts("help:\tprint this help memu\r\n");
    mini_uart_puts("info:\tprint hardware's information\r\n");
    mini_uart_puts("hello:\tprint Hello World!\r\n");
    mini_uart_puts("reboot:\treboot the device\r\n");
}

void hello(void) {
    mini_uart_puts("Hello World!\r\n");
}

void reboot(void) {
    mini_uart_puts("rebooting...\r\n");
    reset(100);
}

void message(char *s) {
    mini_uart_puts(s);
    mini_uart_puts(" command not found\r\n");
}