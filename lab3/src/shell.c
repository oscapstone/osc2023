#include "mini_uart.h"
#include "mailbox.h"
#include "shell.h"
#include "utils.h"
#include "cpio.h"
#include "mem.h"
#include "timer.h"
#include "exception.h"


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
    else if(!strcmp(buffer, "exec"))     cmd_exec();  
    else if(!strcmp(buffer, "boottime")) cmd_boottime();  
    else if(!strcmp(buffer, "async"))    cmd_async();  
    else if(!strcmp(buffer, "multiplex"))cmd_multiplex();  
}

void cmd_help(){
    uart_puts("help\t\t: print this help menu\r\n");
    uart_puts("hello\t\t: print Hello World!\r\n");
    uart_puts("reboot\t\t: reboot the device\r\n");
    uart_puts("hwinfo\t\t: print hardware info\r\n");
    uart_puts("ls\t\t: print filenames in initramfs file\r\n");
    uart_puts("cat\t\t: print the content of file in initramfs file\r\n");
    uart_puts("malloc\t\t: simple test for the simple_malloc function\r\n");
    uart_puts("exec\t\t: run user program in initramfs\r\n");
    uart_puts("boottime\t: get time after booting every two secs\r\n");
    uart_puts("async\t\t: start asynchronous read/write\r\n");
    uart_puts("multiplex\t: test core timer multiplexing function\r\n");
    
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

void cmd_exec(){
    cpio_exec();
}

void cmd_boottime(){
    disable_interrupt();

    core_timer_enable();
    set_timer(2 * get_core_frequency());

    void (*location)(void) = infinite;
    asm volatile(
        "msr     elr_el1, %0\r\n\t"     // Address
        "mov     x0, 0x340\r\n\t"
        "msr     spsr_el1, x0\r\n\t"    // Status(interupt)
        "mov     x0, sp\r\n\t"
        "msr     sp_el0, x0\r\n\t"      // sp
        "eret    \r\n\t"
        ::
        "r" (location)
    );

    infinite();
}

void cmd_async(){

    enable_interrupt();
    enable_uart_interrupt();

    char input[1024];
    while(1){
        async_uart_puts("(a)# ");

        unsigned int input_len = async_uart_gets(input,1024);
        if(input_len==0){
            async_uart_puts("\r\n");
            continue;
        }
        else{
            async_uart_puts(input);
            async_uart_puts("\r\n");
            if(strncmp(input,"exit",input_len)==0){
                async_uart_puts("\r\n");
                break;
            }
        }

    }
    disable_uart_interrupt();
    disable_interrupt();
}

void cmd_multiplex(){
    disable_interrupt();
    core_timer_disable();

    add_core_timer("0st sent, 2rd received.\r\n", 2);
    add_core_timer("1st sent, 4rd received.\r\n", 4);
    add_core_timer("2nd sent, 3nd received.\r\n", 3);
    add_core_timer("3rd sent, 1st received.\r\n", 1);

    enable_interrupt();
    core_timer_enable();

    int enable = 1;

    while (enable) {
        asm volatile("mrs %0, cntp_ctl_el0\r\n" :"=r"(enable));
    }
}