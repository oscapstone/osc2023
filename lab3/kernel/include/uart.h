#ifndef _UART_H
#define _UART_H

void uart_init ( void );
char uart_recv ( void );  //read
void uart_send ( char c ); //write
void uart_send_string(char* str);
void uart_flush(void);
void uart_sendline();

#endif /* _UART_H */

