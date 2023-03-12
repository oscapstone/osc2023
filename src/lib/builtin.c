#include <mini_uart.h>
#include <reboot.h>
#include <mbox.h>
#include <builtin.h>
#include <utils.h>

void _help(int mode){
    uart_printf(
        "help\t: "   "print this help menu" "\r\n"
        "hello\t: "  "print Hello World!"   "\r\n"
        "hwinfo\t: " "get hardware information" "\r\n"
        "reboot\t: " "reboot the device" "\r\n"
    );
    if(mode)
        uart_printf("load\t: " "load the kernel" "\r\n");
    else{
        uart_printf(
            "ls\t: " "list files in initramfs"  "\r\n"
            "cat <filename>\t: " "get file content"  "\r\n"
        );
    }
}

void _hello(void){
    uart_printf("Hello World!\r\n");
}

void _hwinfo(void){
    unsigned int recv;
    arm_info arm_mem;
    get_board_revision(&recv);
    get_arm_memory(&arm_mem);
    if(recv!=0)
        uart_printf("[*] Revision: %x\r\n", recv);
    
    uart_printf("[*] ARM memory base address: %x\r\n", arm_mem.base_addr);
    uart_printf("[*] ARM memory size: %x\r\n", arm_mem.size);
}

void _reboot(void){
    uart_printf("Rebooting...\r\n\r\n");
    delay(10000);
    reset(10);
    while(1);
}

void _echo(char* shell_buf){
    uart_printf(shell_buf);
}