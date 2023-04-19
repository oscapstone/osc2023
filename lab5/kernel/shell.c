#include "shell.h"
#include "mini_uart.h"
#include "uart_async.h"
#include "utils.h"
#include "mailbox.h"
#include "reboot.h"
#include "ramdisk.h"
#include "string_utils.h"
#include "exception.h"
#include "mm.h"
#include "timer.h"
#include "mem_frame.h"
#include "mem_allocator.h"
#include "thread.h"

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
        uart_send_string("exec:\t\texecute file in ramdisk\r\n");
        uart_send_string("demo-async:\tshow uart async send and receive\r\n");
        uart_send_string("set-timeout [msg] [sec]:\r\n");
        uart_send_string("\tprint [msg] after [sec] seconds\r\n");
        uart_send_string("demo-frame:\tdemo frame allocation and free\r\n");
        uart_send_string("demo-malloc:\tdemo malloc and free\r\n");
        uart_send_string("show-reserve:\tshow reserved parts\r\n");
        uart_send_string("demo-thread:\tdemo threading el1\r\n");
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
        } else if (!strcmp(buffer, "exec")) {
                uart_send_string("[INFO] exec: not implemented");
                // TODO: not to copy or allocate a place
                /*
                if (!ramdisk_load_file_to_adr(USER_PROGRAM_START)) return;
                reset_core_timer_in_second(2);
                branch_to_address_el0(USER_PROGRAM_START, USER_STACK_POINTER);
                */
        } else if (!strcmp(buffer, "demo-async")) {
                reset_core_timer_in_second(600);
                branch_to_address_el0(demo_uart_async, USER_STACK_POINTER);
        } else if (!strncmp(buffer, "set-timeout ", strlen("set-timeout "))) {
                cmd_add_timer(buffer);
        } else if (!strcmp(buffer, "demo-frame")) {
                demo_frame();
        } else if (!strcmp(buffer, "demo-malloc")) {
                demo_dynamic_allocation();
        } else if (!strcmp(buffer, "show-reserve")) {
                show_reservation();
        } else if (!strcmp(buffer, "demo-thread")) {
                demo_thread(3);
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