#include "uart.h"
#include "interrupt.h"

int core_timer_enable(void){
	asm volatile(
	"mov x0, #1;"
    	"msr cntp_ctl_el0, x0;" // enable
    	"mrs x0, cntfrq_el0;"
    	"msr cntp_tval_el0, x0;" // set expired time
    	"mov x0, #2;"
    	"ldr x1, =0x40000040;"
    	"str w0, [x1];" // unmask timer interrupt
	);
	uart_puts(" CORE TIMER INITIAL\n");
	return 0;
}

int core_timer_handler(void){
	uart_puts("\n\nTimer handler!\n");
	unsigned long time, freq;
	asm volatile(
		"mrs	%[time], cntpct_el0;"
		"mrs	%[freq], cntfrq_el0;"
		: [time] "=r" (time), [freq] "=r" (freq)
	);
	uart_puts("Time: ");
	uart_hex(time/freq);
    uart_puts("s\n");
	asm volatile(
		"mrs x0, cntfrq_el0;"
		"mov x0, x0, LSL 1;"
		"msr cntp_tval_el0, x0;"
	);
	return 0;
}

int mini_uart_interrupt_enable(void){
	*IRQS1 |= (1<<29);	// Enable mini uart interrupt, connect the GPU IRQ to CORE0's IRQ
	return 0;
}

static unsigned int get_core0_irq_source(void) {
	return *CORE0_IRQ_SOURCE;  // return the source ID of the interrupt.
}

static int clear_core0_irq_source(void) {
	*CORE0_IRQ_SOURCE = 0;
	return 0;
}

static int uart_handler(void){
	// Only care the 2:1 bit in this register.
	switch (*AUX_MU_IIR & 0x6){
	case 2:
		uart_transmit_handler();
		break;
	case 4:
		uart_receive_handler();
		break;
	case 0:
	default:
		uart_puts("Error\n");
		break;
	}
	return 0;
}

int irq_handler(void){
	if(*IRQ_PEND_1 & (1 << 29))
		uart_handler();
	else 
		core_timer_handler();

	uart_puts("IRQ_HANDLER END\n\n");
	
	return 0;
}
