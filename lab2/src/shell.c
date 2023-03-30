#include "mini_uart.h"
#include "mailbox.h"
#include "shell.h"
#include "utils.h"
#include "cpio.h"
#include "mem.h"


void shell_start(){
    char buffer[BUFFER_SIZE];

    while(1){
        uart_puts("# ");
        uart_gets(buffer);
        cmd_handle(buffer);
    }
}

void cmd_handle(char *buffer){
    if(!strcmp(buffer, "help"))          cmd_help();
    else if(!strcmp(buffer, "hello"))    cmd_hello();
    else if(!strcmp(buffer, "reboot"))   cmd_reboot();
    else if(!strcmp(buffer, "hwinfo"))   cmd_hwinfo();
    else if(!strcmp(buffer, "ls"))       cmd_ls();  
    else if(!strcmp(buffer, "cat"))      cmd_cat();  
    else if(!strcmp(buffer, "malloc"))   cmd_malloc();  
}

void cmd_help(){
    uart_puts("help\t: print this help menu\r\n");
    uart_puts("hello\t: print Hello World!\r\n");
    uart_puts("reboot\t: reboot the device\r\n");
    uart_puts("hwinfo\t: print hardware info\r\n");
    uart_puts("ls\t: print filenames in initramfs file\r\n");
    uart_puts("cat\t: print the content of file in initramfs file\r\n");
    uart_puts("malloc\t: simple test for the simple_malloc function\r\n");
}

void cmd_hello(){
    uart_puts("Hello World!\r\n");
}

void cmd_reboot(){
    uart_puts("Rebooting..\r\n");
    *(volatile unsigned int*)PM_WDOG = PM_PASSWORD | 5000;
    *(volatile unsigned int*)PM_RSTC = PM_PASSWORD | 0x20;
    while(1){}
}

void cmd_hwinfo(){
    mbox_get_HW_Revision();
    mbox_get_ARM_MEM();
}

void cmd_ls(){
    cpio_ls();
}

void cmd_cat(){
    cpio_cat();
}

void cmd_malloc(){
    unsigned int size = 5;
    uart_puts("String size:");
    uart_hex(size);
    uart_puts("\r\n");

    char *str = (char*) simple_malloc(5);
    for(int i=0;i<4;i++){
        str[i] = '0' + i;
    }
    str[4] = '\0';
    uart_puts("String:");
    uart_puts(str);
    uart_puts("\r\n");

}