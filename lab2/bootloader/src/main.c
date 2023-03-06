#include "uart1.h"
#include "shell.h"

extern char* _bootloader_relocated_addr;
extern unsigned long long __code_size;
extern unsigned long long _start;

int relocated_flag = 1;

void code_relocate(char* addr)
{
    unsigned long long size = (unsigned long long)&__code_size;
    char* start = (char *)&_start;
    for(unsigned long long i=0;i<size;i++)
    {
        addr[i] = start[i];
    }

    ((void (*)(void))addr)();
}


void main(){
    char* relocated_ptr = (char*)&_bootloader_relocated_addr;

    if (relocated_flag)
    {
        relocated_flag = 0;
        code_relocate(relocated_ptr);
    }

    char input_buffer[CMD_MAX_LEN];

    uart_init();
    cli_print_banner();

    while(1){
        cli_cmd_clear(input_buffer, CMD_MAX_LEN);
        uart_puts("# ");
        cli_cmd_read(input_buffer);
        cli_cmd_exec(input_buffer);
    }
}
