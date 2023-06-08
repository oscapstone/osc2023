#ifndef _UART_H
#define _UART_H

void uart_init ( void );
char uart_recv ( void );  //read
void uart_send ( char c ); //write
void uart_send_string();
void uart_flush(void);
void uart_sendline();
void uart_interrupt_enable();
void uart_interrupt_disable();
void uart_r_irq_handler();
void uart_w_irq_handler();


#endif /* _UART_H */

