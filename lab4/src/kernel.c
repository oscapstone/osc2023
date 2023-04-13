#include "mini_uart.h"
#include "shell.h"
#include "ramdisk.h"
#include "string.h"
#include "fdt.h"
#include "check_interrupt.h"
#include "buddy.h"

extern void set_exception_vector_table();

extern unsigned char _end;
static char* kernel_end = (char*)&_end;

char* cpio_addr(char* value)	//callback func , do ramdisk address get & setup
{
	char* ramdisk_start = value;
	uint32_t ramdisk = to_little_endian_32(*((uint32_t*)ramdisk_start));
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
	
	void (*func)(char*);							//define callback func
	func = &cpio_addr;
	fdt_api((char*)dtb,func,"linux,initrd-start");

	char* ramf_start = 0x08000000;
	char* ramf_end = 0x08000200;
	
	set_exception_vector_table();					//set vbar_el1 for exception & interrupt
   
	init_buddy();									//simple allocator
	memory_reserve(0x0,(char*)0x1000);				//Spin tables for multicore boot
	memory_reserve((char*)0x80000,kernel_end);		//Kernel image in the physical memory
	memory_reserve(ramf_start,ramf_end);			//Initramfs
	memory_reserve(dtb,dtb + 0x16000);				//Devicetree

	while (1)
    {
		char command[100];
        shell_input(command);
        shell_option(command,(char*)ramf_start);
    }
}
