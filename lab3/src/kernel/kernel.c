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
// lab3
#include <exception.h>
#include <coretimer.h>
#include <interrupt.h>

extern uint32_t _rodata_begin;
extern uint32_t _rodata_end;
extern uint32_t _data_begin;
extern uint32_t _data_end;
extern uint32_t _bss_begin;
extern uint32_t _bss_end;
extern uint32_t _heap_begin;
extern uint32_t _heap_end;
extern uint32_t _stack_top;
extern void *_devicetree_begin;

extern uint32_t _user_begin;
extern uint32_t _user_end;

void _init(void){
	for(uint32_t*addr = &_bss_begin; addr != &_bss_end; addr++){
		*addr = 0;
	}
	uart_init();
	boot_message();
	allocator_test();
	devicetree_check();
	exception_init();
    coretimer_el0_enable();
    interrupt_enable();
	spsr_el1_check();
	add_timer(8, &boottimer, 0);
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

	uart_print("&_rodata_begin=");
	uart_print_hex((uint64_t) &_rodata_begin, 64);
	newline();
	uart_print("&_rodata_end=");
	uart_print_hex((uint64_t) &_rodata_end, 64);
	newline();
	uart_print("&_data_begin=");
	uart_print_hex((uint64_t) &_data_begin, 64);
	newline();
	uart_print("&_data_end=");
	uart_print_hex((uint64_t) &_data_end, 64);
	newline();
	uart_print("&_bss_begin=");
	uart_print_hex((uint64_t) &_bss_begin, 64);
	newline();
	uart_print("&_bss_end=");
	uart_print_hex((uint64_t) &_bss_end, 64);
	newline();
	uart_print("&_heap_begin=");
	uart_print_hex((uint64_t) &_heap_begin, 64);
	newline();
	uart_print("&_heap_end=");
	uart_print_hex((uint64_t) &_heap_end, 64);
	newline();
}

void allocator_test(){
	void *test1 = simple_malloc(1);
	if(test1 == NULL){
		uart_print("Error: Get NULL Pointer!");
		newline();
		return;
	}
    void *test2 = simple_malloc(16);
	if(test2 == NULL){
		uart_print("Error: Get NULL Pointer!");
		newline();
		return;
	}
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

void spsr_el1_check(){
    uint64_t spsr_el1 = 0;
    asm("mrs %0, spsr_el1":"=r"(spsr_el1));
    uart_print("spsr_el1: ");
    uart_print_hex(spsr_el1, 32);
	newline();
}

void boottimer(){
    uint32_t cntfrq_el0;
    uint64_t cntpct_el0;
    asm("mrs %0, cntfrq_el0":"=r"(cntfrq_el0));
    asm("mrs %0, cntpct_el0":"=r"(cntpct_el0));
    uart_print("Seconds after booting: ");
    uart_print_hex(cntpct_el0 / cntfrq_el0, 64);
	newline();
    add_timer(8, &boottimer, 0);
}
