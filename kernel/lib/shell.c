#include "uart.h"
#include "string.h"
#include "shell.h"
#include "mailbox.h"
#include "system.h"
#include "exec.h"
#include "cpio.h"
#include "dtb.h"
#include "timer.h"
#include "task.h"
#include "mm.h"

void shell()
{
    char cmd[MAX_BUF_SIZE];
    print_boot_messages();
    while (1)
    {
        uart_async_printf("# ");
        uart_async_gets(cmd);
        cmd_resolve(cmd);
    }
}

void cmd_resolve(char *cmd)
{
    char *argv[100];
    unsigned int argc = 0;
    unsigned int cmd_length = strlen(cmd);
    int i, j;

    argv[0] = smalloc(MAX_BUF_SIZE);
    for (i = 0, j = 0; i < cmd_length; ++i)
    {
        if (cmd[i] == ' ')
        {
            argv[argc][j] = '\0';
            argv[++argc] = smalloc(MAX_BUF_SIZE);
            j = 0;
            continue;
        }
        argv[argc][j++] = cmd[i];
    }
    argv[argc++][j] = '\0';

    if (!strcmp(cmd, "help"))
    {
        uart_async_printf("help                            : print this help menu\n");
        uart_async_printf("hello                           : print Hello World!\n");
        uart_async_printf("mailbox                         : show infos of board revision and ARM memory\n");
        uart_async_printf("reboot                          : reboot the device\n");
        uart_async_printf("ls                              : list directory contents\n");
        uart_async_printf("cat [FILE]                      : concatenate files and print on the standard output\n");
        uart_async_printf("exec [FILE]                     : load program and execute a program\n");
        uart_async_printf("setTimeout [MESSAGE] [SECONDS]  : print message after [SECONDS] seconds\n");
        uart_async_printf("twoSecAlert [MESSAGE]           : set an alarm that alert every two seconds\n");
        uart_async_printf("testPreempt                     : test preemption\n");
        uart_async_printf("malloc [SIZE]                   : memory allocation\n");
        uart_async_printf("testPFA                         : test page frame allocator\n");
        uart_async_printf("testCSA                         : test chunk slot allocator\n");
        uart_async_printf("clear                           : clear page\n");
    }
    else if (!strcmp(argv[0], "hello"))
        uart_async_printf("Hello World!\n");
    else if (!strcmp(argv[0], "reboot"))
        reboot();
    else if (!strcmp(argv[0], "mailbox"))
        print_system_messages();
    else if (!strcmp(argv[0], "ls"))
        ls(".");
    else if (!strcmp(argv[0], "cat"))
        cat(argv[1]);
    else if (!strcmp(argv[0], "exec"))
        execfile(argv[1]);
    else if (!strcmp(argv[0], "setTimeout"))
        add_timer(uart_puts, argv[1], atoi(argv[2]));
    else if (!strcmp(argv[0], "twoSecAlert"))
        add_timer(two_second_alert, argv[1], 2);
    else if (!strcmp(argv[0], "testPreempt"))
        test_preemption();
    else if (!strcmp(argv[0], "malloc"))
        malloc(atoi(argv[1]));
    else if (!strcmp(argv[0], "testPFA"))
        page_frame_allocator_test();
    else if (!strcmp(argv[0], "testCSA"))
        chunk_slot_allocator_test();
    else if (!strcmp(argv[0], "clear"))
        clear();
    else
        uart_async_printf("Unknown command!: %s\n", argv[0]);
}

void clear()
{
    uart_async_printf("\x1b[H\x1b[J");
}

void print_boot_messages()
{
    clear();
    uart_async_printf("WELCOME !!!!!\n");
    uart_async_printf("dtb address: %x\n", dtb_base);
    uart_async_printf("\n");
}

void print_system_messages()
{
    unsigned int board_revision;
    get_board_revision(&board_revision);
    uart_async_printf("Board revision is : 0x%x\n", board_revision);

    unsigned int arm_mem_base_addr;
    unsigned int arm_mem_size;

    get_arm_memory_info(&arm_mem_base_addr, &arm_mem_size);
    uart_async_printf("ARM memory base address in bytes : 0x%x\n", arm_mem_base_addr);
    uart_async_printf("ARM memory size in bytes : 0x%x\n", arm_mem_size);
}