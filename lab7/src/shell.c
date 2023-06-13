#include "uart.h"
#include "string.h"
#include "shell.h"
#include "mbox.h"
#include "system.h"
#include "filesystem.h"
#include "dtb.h"
#include "timer.h"
#include "malloc.h"
#include "sched.h"

void shell()
{
    char cmd[MAX_BUF_SIZE];
    print_system_messages();
    init_thread_sched();
    timer_list_init();
    core_timer_enable();
    while (1)
    {
        uart_async_printf("# ");
        uart_async_gets(cmd);
        do_cmd(cmd);
    }
}

void do_cmd(char *cmd)
{
    if (strcmp(cmd, "help") == 0)
    {
        uart_async_printf("help                            : print this help menu\r\n");
        uart_async_printf("hello                           : print Hello World!\r\n");
        uart_async_printf("reboot                          : reboot the device\r\n");
        uart_async_printf("ls                              : list current directory\r\n");
        uart_async_printf("cat                             : print content of a file\r\n");
        uart_async_printf("show_device_tree                : show device tree\r\n");
        uart_async_printf("exec                            : load a user program in the initramfs and run it in EL0\r\n");
        uart_async_printf("setTimeout [MESSAGE] [SECONDS]  : print message after [SECONDS] seconds (non-blocking)\r\n");
        uart_async_printf("clockAlert                      : alert every two seconds\r\n");
    }
    else if (strcmp(cmd, "hello") == 0)
    {
        uart_async_printf("Hello World!\r\n");
    }
    else if (strcmp(cmd, "reboot") == 0)
    {
        reboot();
    }
    else if (strcmp(cmd, "cat") == 0)
    {
        uart_async_printf("Filename: ");
        char filepath[MAX_BUF_SIZE];
        uart_async_gets(filepath);
        cat(filepath);
    }
    else if (strcmp(cmd, "ls") == 0)
    {
        ls(".");
    }
    else if (strcmp(cmd, "show_device_tree") == 0)
    {
        traverse_device_tree(dtb_place, dtb_callback_show_tree);
    }
    else if (strcmp(cmd, "exec") == 0) // in filesystem.c
    {
        uart_async_printf("Filename: ");
        char filepath[MAX_BUF_SIZE];
        uart_async_gets(filepath);
        execfile(filepath);
    }
    else if (strncmp(cmd, "setTimeout", sizeof("setTimeout") - 1) == 0)
    {
        strchr(cmd, ' ');
        char *message = strchr(cmd, ' ');
        if (!message)
        {
            uart_async_printf("setTimeout wrong format");
            return;
        }
        message += 1;
        char *end_message = strchr(message, ' ');
        if (!end_message)
        {
            uart_async_printf("setTimeout wrong format");
            return;
        }
        *end_message = '\0';
        char *seconds = end_message + 1;
        add_timer(uart_puts, atoi(seconds), message,0);
    }
    else if (strcmp(cmd, "clockAlert") == 0)
    {
        add_timer(two_second_alert, 2, "two_second_alert",0);
    }
    else
    {
        uart_async_printf("Unknown command!: %s\r\n", cmd);
    }
}

void print_system_messages()
{
    unsigned int board_revision;
    get_board_revision(&board_revision);
    uart_async_printf("Board revision is : 0x%x\r\n", board_revision);

    unsigned int arm_mem_base_addr;
    unsigned int arm_mem_size;

    get_arm_memory_info(&arm_mem_base_addr, &arm_mem_size);
    uart_async_printf("ARM memory base address in bytes : 0x%x\r\n", arm_mem_base_addr);
    uart_async_printf("ARM memory size in bytes : 0x%x\r\n", arm_mem_size);
}