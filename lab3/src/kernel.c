#include "mini_uart.h"
#include "shell.h"
#include "ramdisk.h"
#include "string.h"
#include "fdt.h"

extern void set_exception_vector_table();
extern void core_timer_enable();
static uint32_t ramdisk;

void cpio_addr(char* value)	//callback func , do ramdisk address get & setup
{
	char* ramdisk_start = value;
	ramdisk = to_little_endian_32(*((uint32_t*)ramdisk_start));
	init_rd((char*)ramdisk);
}

void kernel_main(void* dtb)		//x0 is the first argument
{
    uart_init();
    uart_send_string("*************************\r\n");
    uart_send_string("Hello from raspi!\r\n");
    uart_send_string("*************************\r\n");

	unsigned long el;
	asm volatile("mrs %0,CurrentEL" : "=r"(el));	//mrs : read system register
	uart_send_string("current EL is : ");
	uart_hex((el>>2)&3);
	uart_send_string("\r\n");
	
	void (*func)(char*);	//define callback func
	func = &cpio_addr;
	fdt_api((char*)dtb,func,"linux,initrd-start");	//find initrd-start property & get its value
	
	set_exception_vector_table();					//set vbar_el1 for exception & interrupt
    
	while (1)
    {
		char command[100];
        shell_input(command);
        shell_option(command,(char*)ramdisk);
    }
}
