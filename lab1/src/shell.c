#include "mini_uart.h"
#include "mailbox.h"
#include "shell.h"


void shell_start(){
    // initail buffer size
    char buffer[BUFFER_SIZE];
    int cur_size = 0;

    while(1){
        uart_send_string("# ");
        cmd_init(buffer, &cur_size);
        cmd_read(buffer, &cur_size);
    }
}

void cmd_init(char *buffer, int *size){
    while(*size){
        (*size)--;
        buffer[(*size)] = '\0';
    }
}

void cmd_read(char *buffer, int *size){
    char c;
    while(1){
        c = uart_recv();

        // int tem = c;
        // while(tem != 0){
        //     uart_send(tem%10 + '0');
        //     tem = tem/10;
        // }

        if(c == '\n' || c == '\r'){
            uart_send_string("\r\n");
            cmd_handle(buffer, size);
            break;
        }
        else if(c == BACKSPACE || c == DEL){
            if((*size) > 0){
                uart_send('\b');
                uart_send(' ');
                uart_send('\b');
                buffer[--(*size)] = '\0';
            }
        }
        else{
            buffer[(*size)++] = c;
            uart_send(c);
        }
    }
}

void cmd_handle(char *buffer, int *size){
    if(!strcmp(buffer, "help"))          cmd_help();
    else if(!strcmp(buffer, "hello"))    cmd_hello();
    else if(!strcmp(buffer, "reboot"))   cmd_reboot();
    else if(!strcmp(buffer, "hwinfo")){  mbox_get_HW_Revision();mbox_get_ARM_MEM();}     
}

void cmd_help(){
    uart_send_string("help\t: print this help menu\r\n");
    uart_send_string("hello\t: print Hello World!\r\n");
    uart_send_string("reboot\t: reboot the device\r\n");
}
void cmd_hello(){
    uart_send_string("Hello World!\r\n");
}
void cmd_reboot(){
    *(volatile unsigned int*)PM_WDOG = PM_PASSWORD | 0x20;
    *(volatile unsigned int*)PM_RSTC = PM_PASSWORD | 100;
}

int strcmp(char *s1, char *s2)
{
    int i;
    for (i = 0; s1[i] != '\0'; i ++)
    {
        if ( s1[i] != s2[i])
        {
            return s1[i] - s2[i];
        }
    }

    return  s1[i] - s2[i];
}