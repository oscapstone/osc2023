#include "shell.h"
#include "string.h"
#include "mini_uart.h"
#include "reboot.h"
#include "mailbox.h"
#include <stddef.h>


#define buffer_max_size 256


void shell_start(){
    while(1){
        char buffer[buffer_max_size];

        uart_display("#");
        read_commad(buffer);
        do_command(buffer);
    }
}

void read_commad(char *buffer){
    size_t index=0;
    while(1){
        buffer[index] = uart_recv();
        uart_send(buffer[index]);  
        if(buffer[index]=='\n'){
            break;
        }   
        index++;
    }
    buffer[index+1]='\0';
}

void help(){
    uart_display("help      : print this help menu\r\n");
    uart_display("hello     : print Hello World!\r\n");
    uart_display("reboot    : reboot the device\r\n");
    uart_display("hw_info   : board version & ARM memory base address and size\r\n");
}

void hello(){
    uart_display("Hello World!\r\n");
}

void do_command(char *buffer){
    str_changend(buffer);

    uart_send('\r');

    if(buffer[0]=='\r'){
        return;
    }

    else if(strcmp(buffer,"help")==0){
        help();
    }

    else if(strcmp(buffer,"hello")==0){
        hello();
    }

    else if(strcmp(buffer,"reboot")==0){
        uart_display("rebooting now ....\r\n");
        reset(1000000000);
    }

    else if(strcmp(buffer,"hw_info")==0){
        board_revision();
        arm_memory();
    }

    else{
        uart_display("*******************\r\n");
        uart_display("command : <");
        uart_display(buffer);
        uart_display("> not found\r\n");
        uart_display("*******************\r\n");
    }

}






