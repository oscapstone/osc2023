#include "uart.h"
#include "string.h"
#include "shell.h"
#include "mbox.h"
#include "system.h"
#include "filesystem.h"
#include "dtb.h"

void shell()
{
    char cmd[MAX_BUF_SIZE];
    print_system_messages();
    while(1)
    {
        uart_printf("# ");
        uart_gets(cmd);
        do_cmd(cmd);
    }
}

void do_cmd(char* cmd)
{
    if(strcmp(cmd,"help")==0)
    {
        uart_puts("help             : print this help menu");
        uart_puts("hello            : print Hello World!");
        uart_puts("reboot           : reboot the device");
        uart_puts("ls               : list current directory");
        uart_puts("cat              : print content of a file");
        uart_puts("show_device_tree : show device tree");
    }
    else if(strcmp(cmd,"hello")==0)
    {
        uart_puts("Hello World!");
    }
    else if(strcmp(cmd,"reboot")==0)
    {
        reboot();
    }else if(strcmp(cmd,"cat")==0)
    {
        uart_printf("Filename: ");
        char filepath[MAX_BUF_SIZE];
        uart_gets(filepath);
        cat(filepath);

    }else if(strcmp(cmd,"ls")==0)
    {
        ls(".");
    }else if(strcmp(cmd,"show_device_tree")==0)
    {
        traverse_device_tree(dtb_place,dtb_callback_show_tree);
    }else
    {
        uart_puts("Unknown command!");
    }
}

void print_system_messages()
{
    unsigned int board_revision;
    get_board_revision(&board_revision);
    uart_printf("Board revision is : 0x");
    uart_hex(board_revision);
    uart_puts("");
    
    unsigned int arm_mem_base_addr;
    unsigned int arm_mem_size;

    get_arm_memory_info(&arm_mem_base_addr,&arm_mem_size);
    uart_printf("ARM memory base address in bytes : 0x");
    uart_hex(arm_mem_base_addr);
    uart_puts("");
    uart_printf("ARM memory size in bytes : 0x");
    uart_hex(arm_mem_size);
    uart_puts("");
}