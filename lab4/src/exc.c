#include "exc.h"
#include "peripherals/mini_uart.h"
enum stat{
	tx_enable=0x2,
	rx_enable=0x4
};

void enable_mini_uart_interrupt(){
	// IER=1 for receive(get) interrupt, 2 for transmit (send) interrupt“
	*IRQs1 |= (1<<29);	
	*AUX_MU_IER_REG = 0x1;
}

void sp_elx_handler(){
	if (*IRQs1 & (1<<29)){
		int uart = *AUX_MU_IIR_REG & 0b110;
		if (uart==tx_enable){
			uart_transmit_handler();
		} else if (uart==rx_enable) {
			uart_receive_handler();
		} else {
			uart_puts("Asynchronous IO anomally!!!");
		}
	}
	else {
		asm volatile(
			"mrs x0, cntfrq_el0;"
			"mrs x1, cntpct_el0;"
			"stp lr, x0, [sp, #-16]!;"
			"bl print_core_timer;"
			"ldp lr, x0, [sp], #16;"
			"lsl x0, x0, #1;"
			"msr cntp_tval_el0, x0;"
			"ret;");
	}
}

/**
 * As the refrence from BCM2835 p.12, AUX_MU_IER_REG:
 * Bit 1 represents the "rx" bit,
 * Bti - represents the "tx" bit.
 * Following 4 operations encapsulated the enable/disable operations.
 */
void disable_tx(){
	*AUX_MU_IER_REG &= 0x1;
}

void disable_rx(){
	*AUX_MU_IER_REG &= 0x2;
}

void enable_tx(){
	*AUX_MU_IER_REG |= 0x1;
}

void exable_rx(){
	*AUX_MU_IER_REG |= 0x2;
}
