#ifndef _EXC_C
#define _EXC_C

void enable_mini_uart_interrupt();
void sp_elx_handler();
void disable_tx();
void disable_rx();
void enable_tx();
void enable_rx();

#endif
