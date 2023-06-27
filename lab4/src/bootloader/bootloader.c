#include <bootloader.h>

#include <stdint.h>
#include <stddef.h>

#include <uart.h>
#include <utils.h>
#include <string.h>

extern uint32_t _bss_begin;
extern uint32_t _bss_end;
extern uint32_t _stack_top;

void _init(void){
	for(uint32_t*addr = &_bss_begin; addr != &_bss_end; addr++){
		*addr = 0;
	}
	uart_init();
	load_kernel();
}

void load_kernel(){
	char *kernel_addr = (char *)0x80000;
	uart_print("Input Size: ");
	char buf[BUF_SIZE];
	size_t recvlen = uart_readline(buf);
	buf[recvlen] = '\0';
	uint32_t imagesize = atoi(buf);
	uart_print("Input Kernel Image (Size: ");
	uart_print_hex(imagesize, 32);
	uart_print("): ");
	uart_read(kernel_addr, imagesize);
	uart_print("Go to kernel!");
	newline();
	asm volatile(
		"mov x0, x10\n"
		"ldr x1, =0x80000\n"
    	"br x1\n"
    );
}