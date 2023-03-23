#ifndef	_MINI_UART_H
#define	_MINI_UART_H



extern volatile unsigned int mbox[36];

void uart_init ( void );
char uart_getc ( void );
void uart_send(char c);
void uart_puts(char *s);
void uart_puts_l(char *s, int l);
void uart_hex(unsigned int d);
void uart_hexlong(unsigned long d);
void uart_hexdump(unsigned int d);
void set(long addr, unsigned int value);
void reset(int tick);
void cancel_reset();
void mbox_call(unsigned char ch);
int memcmp(void *s1, void *s2, int n);

#endif  /*_MINI_UART_H */