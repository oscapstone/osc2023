#include "exception.h"
#include "mini_uart.h"
#include "mailbox.h"
#include "utils.h"
#include "timer.h"
#include "peripherals/exception.h"
#include "peripherals/mini_uart.h"

void enable_interrupt() {asm volatile("msr DAIFClr, 0xf");}
void disable_interrupt() {asm volatile("msr DAIFSet, 0xf");}

const char *entry_error_messages[] = {
	"SYNC_INVALID_EL1t",
	"IRQ_INVALID_EL1t",		
	"FIQ_INVALID_EL1t",		
	"ERROR_INVALID_EL1t",		

	"SYNC_INVALID_EL1h",		
	"IRQ_INVALID_EL1h",		
	"FIQ_INVALID_EL1h",		
	"ERROR_INVALID_EL1h",		

	"SYNC_INVALID_EL0_64",		
	"IRQ_INVALID_EL0_64",		
	"FIQ_INVALID_EL0_64",		
	"ERROR_INVALID_EL0_64",	

	"SYNC_INVALID_EL0_32",		
	"IRQ_INVALID_EL0_32",		
	"FIQ_INVALID_EL0_32",		
	"ERROR_INVALID_EL0_32",

	"SYNC_ERROR",
	"SYSCALL_ERROR"	
};

void show_invalid_entry_message(int type, unsigned long esr, unsigned long addr) {
	printf("%s, ESR: 0x%x, address: 0x%x\n", entry_error_messages[type], esr, addr);
}

void handle_irq() {
	
	if (get32(CORE0_INTERRUPT_SRC)&INTERRUPT_SOURCE_CNTPNSIRQ) {
		handle_timer_irq();
	} else {
		printf("unknown irq encountered");
	}

}