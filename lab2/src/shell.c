#include "mini_uart.h"
#include "utils.h"
#include "shell.h"
#include "str_tool.h"
#include "stdint.h"
#include "mem.h"
#include "cpio.h"

#define MAX_INPUT 100

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024


struct CMD {
    char *name;
    char *help;
    void (*func)(void);
};
struct CMD command[] = {
    {.name="hello", .help="print Hello World!", .func=shell_hello},
    {.name="help", .help="print all available commands", .func=shell_help},
    {.name="reboot", .help="reboot the machine", .func=shell_reboot},
    {.name="ls", .help="list all the file", .func=shell_ls},
    {.name="cat", .help="list all the file content", .func=shell_cat},
    {.name="allocator", .help="get the pointer of address", .func=shell_malloc}
};

char input_buffer[MAX_INPUT+1];
int input_tail_idx = 0;

void buffer_clear(){
    input_tail_idx = 0;
    input_buffer[input_tail_idx] = 0;
}

void init_shell(){
    uart_puts("Welcome! Initiating shell...\n");
    buffer_clear();
}

void execute_cmd(char* input_cmd){
    int cmd_len = sizeof(command)/sizeof(struct CMD);
    int executed=0;
    for(int i=0;i<cmd_len;i++){
        if(!strcmp(input_cmd, command[i].name)){
            executed=1;
            command[i].func();
            break;
        }
    }
    if(!executed){
        uart_puts("Command not found!\n");
    }
}

void print_input_prompt(){
    uart_puts(">> ");
}

void get_input(){
    char cur_char;
    while(1){
        cur_char = uart_getc();
        if(cur_char=='\r'){
            uart_puts("\r\n");
            break;
        }
        else if(cur_char == 127){
            del_key();
        }
        else{
            if(input_tail_idx==MAX_INPUT){
                uart_puts("Input string is too long!\n");
                break;
            }
            uart_putc(cur_char);
            input_buffer[input_tail_idx]=cur_char;
            input_tail_idx++;
        }
    }
    input_buffer[input_tail_idx] = 0;
}

void del_key(){
    if(input_tail_idx>0){
        input_tail_idx -= 1;
        uart_puts("\033[1D");
        uart_puts(" ");
        uart_puts("\033[1D");
    }
}

void simple_shell(){
    init_shell();
    while(1){
        print_input_prompt();
        get_input();
        execute_cmd(input_buffer);
        buffer_clear();
    }
}

void shell_hello(){
    uart_puts("Hello world!\n");
}

void shell_malloc(){
    char *str=(char*)simple_malloc(8);
    for(int i =0;i<8;i++){
        if(i!=7){
            str[i]='A'+i;
        }
        else{
            str[i]='\0';
        }
    }
    uart_puts(str);
    uart_puts("\r\n");
}


void shell_help(){
    uart_puts("===============================================");
    uart_puts("\r\n");
    uart_puts("Command Name");
    uart_puts("\t");
    uart_puts("Description");
    uart_puts("\r\n");
    uart_puts("===============================================");
    uart_puts("\r\n");

    int cmd_len = sizeof(command)/sizeof(struct CMD);
    for(int cmd_idx=0; cmd_idx<cmd_len; cmd_idx+=1){
        uart_puts(command[cmd_idx].name);
        uart_puts("\t\t");
        uart_puts(command[cmd_idx].help);
        uart_puts("\r\n");
    }
    uart_puts("===============================================");
    uart_puts("\r\n");
}


void shell_reboot(){
    uart_puts("Reboot after 10 watchdog tick!\r\n");
    delay(100000);
    put32(PM_RSTC, PM_PASSWORD | 0x20);
    put32(PM_WDOG, PM_PASSWORD | 10);
}

void shell_ls(){
    cpio_list();

}

void shell_cat(){
    cpio_concatenate();
}