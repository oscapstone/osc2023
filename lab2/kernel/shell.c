#include "shell.h"
#include "mini_uart.h"
#include "utils.h"
#include "mailbox.h"
#include "reboot.h"
#include "ramdisk.h"
#include "string_utils.h"

#define MAX_BUFFER_SIZE 256u
static char buffer[MAX_BUFFER_SIZE];

void send_help_message(void)
{
        uart_send_string("help:\t\tprint this help menu.\r\n");
        uart_send_string("hello:\t\tprint Hello World!\r\n");
        uart_send_string("reboot:\t\treboot device\r\n");
        uart_send_string("hw-info:\tprint hardware information\r\n");
        uart_send_string("ls:\t\tlist files in ramdisk\r\n");
        uart_send_string("cat:\t\tprint file\r\n");
}

void parse_cmd(void)
{
        if (!strcmp(buffer, "\0")) {
                uart_send_string("\r\n");
        } else if (!strcmp(buffer, "hello")) {
                uart_send_string("Hello World!\r\n");
        } else if (!strcmp(buffer, "help")) {
                send_help_message();
        } else if (!strcmp(buffer, "hw-info")) {
                get_board_revision();
                get_arm_memory();
        } else if (!strcmp(buffer, "reboot")) {
                uart_send_string("rebooting ...\r\n");
                reset(100);
        } else if (!strcmp(buffer, "ls")) {
                ramdisk_ls();
        } else if (!strcmp(buffer, "cat")) {
                ramdisk_cat();
        } else {
                uart_send_string("Command not found, ");
                uart_send_string("type 'help' for commands.\r\n");
        }
}

void shell_loop(void)
{
        while (1) {
                uart_send_string("# ");
                uart_readline(buffer, MAX_BUFFER_SIZE);
                parse_cmd();
        }
}