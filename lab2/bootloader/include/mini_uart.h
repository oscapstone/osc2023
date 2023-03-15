#ifndef	_MINI_UART_H
#define	_MINI_UART_H



extern volatile unsigned int mbox[36];

void uart_init ( void );
char uart_getc ( void );
void uart_send(char c);
void uart_puts(char *s);
void uart_hex(unsigned int d);
void uart_hexdump(unsigned int d);
int atoi(char * ch);
void uart_int(int i);
int exp(int i, int j);

#endif  /*_MINI_UART_H */