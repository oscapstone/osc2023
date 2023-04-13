#ifndef	_MINI_UART_H
#define	_MINI_UART_H



extern volatile unsigned int mbox[36];

void uart_init ( void );
char uart_getc ( void );
void uart_send(char c);
void uart_puts(char *s);
void uart_async_send(char * str, int len);
void uart_async_getc(char * str, int len);
void uart_transmit_handler();
void uart_receive_handler();
void uart_puts_l(char *s, int l);
void uart_hex(unsigned int d);
void uart_hexlong(unsigned long d);
void uart_hexdump(unsigned int d);
void uart_ulong(unsigned long i);
void set(long addr, unsigned int value);
void reset(int tick);
void cancel_reset();
void mbox_call(unsigned char ch);
int memcmp(void *s1, void *s2, int n);
void delay(int sec);

#endif  /*_MINI_UART_H */
