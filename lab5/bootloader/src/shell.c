#include "uart.h"
#include "string.h"
#include "shell.h"
#include "mbox.h"
#include "system.h"

void shell()
{
    char cmd[MAX_BUF_SIZE];
    print_system_messages();
    uart_puts("Welcome, this is bootloader. Try to load kernel with uart with protocol in system.c(load_kernel)");
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
        uart_puts("help       : print this help menu");
        uart_puts("hello      : print Hello World!");
        uart_puts("reboot     : reboot the device");
        uart_puts("load_kernel   : load kernel code from uart to 0x80000 and jump to it!");
    }
    else if(strcmp(cmd,"hello")==0)
    {
        uart_puts("Hello World!");
    }
    else if(strcmp(cmd,"reboot")==0)
    {
        reboot();
    }else if(strcmp(cmd,"load_kernel")==0)
    {
        load_kernel();
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