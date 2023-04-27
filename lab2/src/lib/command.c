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

void help(){
    uart_print("help\t: print this help menu");
    newline();
    uart_print("hello\t: print Hello World!");
    newline();
    uart_print("reboot\t: reboot the device");
    newline();
    uart_print("ls\t: list directory contents");
    newline();
    uart_print("cat\t: concatenate files and print on the standard output");
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