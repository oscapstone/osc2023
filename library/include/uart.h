void uart_init();
void uart_write(unsigned int character);
char uart_read();
void uart_puts(char *string);
void uart_hex(unsigned int d);
void uart_newline();