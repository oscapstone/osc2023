#include "shell.h"
#include "uart.h"

extern char* __bootloader_relocated_addr;
extern unsigned long long __code_size;
extern unsigned long long _start;
char* _dtb;

int relocated_flag = 1;

void relocate(char* addr) {
    unsigned long kernel_size = (unsigned long long)&__code_size;
	char* start = (char *)&_start;
	for(unsigned long long i=0;i<kernel_size;i++){
		addr[i] = start[i];
	}
	void (*start_os)(char*) = (void *)addr;
    start_os(_dtb);
}

void main(char *arg)
{
	_dtb = arg;
	char* relocate_ptr = (char*)&__bootloader_relocated_addr;
	// relocate once
	if (relocated_flag){
		relocated_flag = 0;
		uart_printf("relocating...");
		relocate(relocate_ptr);
	}
	
	char input_buffer[CMD_MAX_LEN];
    shell_init();
	while(1) {
		cli_cmd_clear(input_buffer, CMD_MAX_LEN);
		
		shell_input(input_buffer);
		shell_controller(input_buffer);
	}
}