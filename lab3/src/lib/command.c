#include <command.h>
// C
#include <stdint.h>
#include <stddef.h>
// tool
#include <utils.h>
#include <string.h>
// lab1
#include <uart.h>
#include <mailbox.h>
#include <reboot.h>
// lab2
#include <allocator.h>
#include <ramdisk.h>
#include <devicetree.h>
// lab3
#include <exception.h>
#include <coretimer.h>
#include <interrupt.h>

extern uint32_t _user_begin;
extern uint32_t _user_end;

void help(){
    uart_print("help\t      : print this help menu");
    newline();
    uart_print("hello\t     : print Hello World!");
    newline();
    uart_print("reboot\t    : reboot the device");
    newline();
    uart_print("ls\t        : list directory contents");
    newline();
    uart_print("cat\t       : concatenate files and print on the standard output");
    newline();
    uart_print("exec\t      : execute a file");
    newline();
    uart_print("setTimeout\t: set a timer");
    newline();
}

void hello(){
    uart_print("Hello World!");
    newline();
}

void reboot(){
    uart_print("Reboot system now!");
    newline();
    reset(1<<16);
}

void ls(){
    cpio_file* file_ptr = cpio_parse();
    for(cpio_file* cur = file_ptr; cur != NULL; cur = cur->next){
        uart_print(cur->name);
        newline();
    }
}

void cat(char *filename){
    cpio_file* file_ptr = cpio_parse();
    if(file_ptr == NULL){
        uart_print("Error: cpio");
        return;
    }
    int isfound = 0;
    for(cpio_file* cur = file_ptr; cur != NULL; cur = cur->next){
        if(strcmp(filename, cur->name) == 0){
            uart_write(cur->content, cur->filesize);
            isfound = 1;
            break;
        }
    }
    if(!isfound){
        uart_print("Not Found!");
        newline();
    }
}

void exec(char *filename){
    cpio_file* file_ptr = cpio_parse();
    if(file_ptr == NULL){
        uart_print("Error: cpio");
        return;
    }
    int isfound = 0;
    for(cpio_file* cur = file_ptr; cur != NULL; cur = cur->next){
        if(strcmp(filename, cur->name) == 0){
            isfound = 1;
            memcpy(&_user_begin, cur->content, cur->filesize);
            coretimer_el0_enable();
            /*
            uart_print("&_user_begin=");
            uart_print_hex((uint64_t) &_user_begin, 64);
            newline();
            uart_print("&_user_end=");
            uart_print_hex((uint64_t) &_user_end, 64);
            newline();
            */
            asm("msr spsr_el1, %0"::"r"((uint64_t)0x0)); // 0x0 enable all interrupt
            asm("msr elr_el1, %0"::"r"(&_user_begin));
            asm("msr sp_el0, %0"::"r"(&_user_end));
            asm("eret");
            break;
        }
    }
    if(!isfound){
        uart_print("Not Found!");
        newline();
    }
}

void timeout(void *arg){
    uart_print("Timeout: ");
    uart_print((char *)arg);
    newline();
}

void setTimeout(){
    int time;
    char *msg = (char *)simple_malloc(0x20);
    char time_inp[0x20];
    uart_print("Time (secs): ");
    uart_readline(time_inp);
    time = atoi(time_inp);
    uart_print("Msg: ");
    uart_readline(msg);
    add_timer(time, &timeout, (void *)msg);
}