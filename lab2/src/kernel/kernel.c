#include <kernel.h>
// C
#include <stdint.h>
#include <stddef.h>
// tool
#include <shell.h>
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

extern uint32_t _bss_begin;
extern uint32_t _bss_end;
extern uint32_t _stack_top;
extern void *_devicetree_begin;

void _init(void){
	for(uint32_t*addr = &_bss_begin; addr != &_bss_end; addr++){
		*addr = 0;
	}
	uart_init();
	boot_message();
	allocator_test();
	devicetree_check();
	shell();
}

void boot_message(){
	char buf[BUF_SIZE];
	uart_print("Boot Success!");
	newline();
	uint32_t board_revision;
    uint32_t arm_memory_base_addr;
    uint32_t arm_memory_size;
	get_board_revision(&board_revision);
    get_ARM_memory_base_address_and_size(&arm_memory_base_addr, &arm_memory_size);

	uart_print("board_revision: ");
	uart_print_hex(board_revision, 32);
	newline();

	uart_print("arm_memory_base_addr: ");
	uart_print_hex(arm_memory_base_addr, 32);
	newline();

	uart_print("arm_memory_size: ");
	uart_print_hex(arm_memory_size, 32);
	newline();
}

void allocator_test(){
	void *test1 = simple_malloc(1);
    void *test2 = simple_malloc(16);

    uart_print("Test Simple Allocator 1: ");
    uart_print_hex((uint64_t)test1, 64);
	newline();

    uart_print("Test Simple Allocator 2: ");
    uart_print_hex((uint64_t)test2, 64);
	newline();
}

void devicetree_check(){
	uart_print("Devicetree address: ");
    uart_print_hex((uint64_t)_devicetree_begin, 64);
	newline();
}