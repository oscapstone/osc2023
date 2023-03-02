#include <kernel.h>

#include <stdint.h>
#include <stddef.h>

#include <uart.h>
#include <utils.h>
#include <mailbox.h>

extern uint32_t _bss_begin;
extern uint32_t _bss_end;
extern uint32_t _stack_top;

//
#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

void set(long addr, unsigned int value) {
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

void reset(int tick) {                 // reboot after watchdog timer expire
    set(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    set(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}

void cancel_reset() {
    set(PM_RSTC, PM_PASSWORD | 0);  // full reset
    set(PM_WDOG, PM_PASSWORD | 0);  // number of watchdog tick
}
//

void _init(void){
	for(uint32_t*addr = &_bss_begin; addr != &_bss_end; addr++){
		*addr = 0;
	}
	uart_init();
	boot_message();
	shell();
}

void boot_message(){
	char buf[BUF_SIZE];
	uart_print("Boot Success!\r\n");

	uint32_t board_revision;
    uint32_t arm_memory_base_addr;
    uint32_t arm_memory_size;
	get_board_revision(&board_revision);
    get_ARM_memory_base_address_and_size(&arm_memory_base_addr, &arm_memory_size);

	uart_print("board_revision: ");
	uart_print_hex(board_revision, 32);
	uart_print("\r\n");

	uart_print("arm_memory_base_addr: ");
	uart_print_hex(arm_memory_base_addr, 64);
	uart_print("\r\n");

	uart_print("arm_memory_size: ");
	uart_print_hex(arm_memory_size, 32);
	uart_print("\r\n");
}

void shell(){
	char buf[BUF_SIZE];
	while(1){
		uart_print("# ");
		uart_readline(buf);
		if(strncmp(buf, "help", 4) == 0){
			uart_print("help\t: print this help menu\r\n");
			uart_print("hello\t: print Hello World!\r\n");
			uart_print("reboot\t: reboot the device\r\n");
		}else if(strncmp(buf, "hello", 5) == 0){
			uart_print("Hello World!\r\n");
		}else if(strncmp(buf, "reboot", 6) == 0){
			uart_print("Reboot system now!\r\n");
			reset(1<<16);
		}else{
			uart_print("Unknown command: [");
			uart_print(buf);
			uart_print("].\r\n");
		}
	}
}
