#include <mini_uart.h>
#include <reboot.h>
#include <mbox.h>
#include <builtin.h>
#include <utils.h>

void _help(void){
    uart_send_string(
        "help\t: "   "print this help menu" "\r\n"
        "hello\t: "  "print Hello World!"   "\r\n"
        "hwinfo\t: " "get hardware information" "\r\n"
        "reboot\t: " "reboot the device" "\r\n"
    );
}

void _hello(void){
    uart_send_string("Hello World!\r\n");
}

void _hwinfo(void){
    unsigned int recv;
    arm_info arm_mem;
    get_board_revision(&recv);
    get_arm_memory(&arm_mem);
    if(recv!=0){
        uart_send_string("[*] Revision: ");
        uart_send_hex(recv);
        uart_send_string("\r\n");
    }            
    uart_send_string("[*] ARM memory base address: ");
    uart_send_hex(arm_mem.base_addr);
    uart_send_string("\r\n");
    uart_send_string("[*] ARM memory size: ");
    uart_send_hex(arm_mem.size);
    uart_send_string("\r\n");
}

void _reboot(void){
    uart_send_string("Rebooting...\r\n\r\n");
    delay(10000);
    reset(10);
    while(1);
}

void _echo(char* shell_buf){
    uart_send_string(shell_buf);
}