#include "uart.h"

char tx_buffer[MAX_BUF_SIZE];
char rx_buffer[MAX_BUF_SIZE];
static unsigned int tx_bf_r_idx;
static unsigned int tx_bf_w_idx;
static unsigned int rx_bf_r_idx;
static unsigned int rx_bf_w_idx;

void init_uart_async_index() {
    tx_bf_r_idx = 0;
    tx_bf_w_idx = 0;
    rx_bf_r_idx = 0;
    rx_bf_w_idx = 0;
}

void uart_init()
{
    register unsigned int r;

    /* map UART1 to GPIO pins */
    r = *GPFSEL1;
    r &= ~((7 << 12) | (7 << 15)); // clean gpio14, gpio15
    r |= (2 << 12) | (2 << 15);    // set alt5 for gpio14, gpio15
    *GPFSEL1 = r;
    *GPPUD = 0; // disable pull up/down for gpio14, gpio15
    r = 150;
    while (r--) // wait 150 cycles
        asm volatile("nop");
    *GPPUDCLK0 = (1 << 14) | (1 << 15); // Set Clock on line 14, 15
    r = 150;
    while (r--) // wait 150 cycles
        asm volatile("nop");
    *GPPUDCLK0 = 0;   // flush GPIO setup

    /* initialize UART */
    *AUX_ENABLE |= 1;   // 1.Set AUXENB register to enable mini UART. Then mini UART register can be accessed.
    *AUX_MU_CNTL = 0;   // 2.Set AUX_MU_CNTL_REG to 0. Disable transmitter and receiver during configuration.
    *AUX_MU_IER = 0;    // 3.Set AUX_MU_IER_REG to 0. Disable interrupt because currently you don’t need interrupt.
    *AUX_MU_LCR = 3;    // 4.Set AUX_MU_LCR_REG to 3. Set the data size to 8 bit.
    *AUX_MU_MCR = 0;    // 5.Set AUX_MU_MCR_REG to 0. Don’t need auto flow control.
    *AUX_MU_BAUD = 270; // 6.Set AUX_MU_BAUD to 270. Set baud rate to 115200
    *AUX_MU_IIR = 0x6;  // 7.Set AUX_MU_IIR_REG to 6. No FIFO. (not clear FIFO ???)
    *AUX_MU_CNTL = 3;   // 8.Set AUX_MU_CNTL_REG to 3. Enable the transmitter and receiver.
}

void uart_putc(char c)
{
    /* wait until we can send */
    do
    {
        asm volatile("nop");
    } while (!(*AUX_MU_LSR & 0x20));
    /* write the character to the buffer */
    *AUX_MU_IO = (unsigned int)c;
}

char uart_getc()
{
    char r;
    /* wait until something is in the buffer */
    do
    {
        asm volatile("nop");
    } while (!(*AUX_MU_LSR & 0x01));
    /* read it and return */
    r = (char)(*AUX_MU_IO);

    /* echo back */
    if (r == '\r')
    {
        uart_printf("\r\r\n");
        do
        {
            asm volatile("nop");
        } while (!(*AUX_MU_LSR & 0x40)); //wait for output success Transmitter idle
    }
    else if (r == '\x7f') // backspace
        uart_printf("\b \b");
    else
        uart_putc(r);
    /* convert carriage return to newline */
    return r == '\r' ? '\n' : r;
}

/**
 * Display a string
 */
void uart_puts(char *s) {
    while(*s) {
        /* convert newline to carriage return + newline */
        if(*s=='\n')
            uart_putc('\r');
        uart_putc(*s++);
    }
}

char *uart_gets(char *buf)
{
    int count;
    char c;
    char *s;
    for (s = buf, count = 0; (c = uart_getc()) != '\n' && count != MAX_BUF_SIZE - 1; count++)
    {
        *s = c;
        if (*s == '\x7f')
        {
            count--;
            if (count == -1)
            {
                uart_putc(' '); // prevent back over command line #
                continue;
            }
            s--;
            count--;
            continue;
        }
        s++;
    }
    *s = '\0';
    return buf;
}

int uart_printf(char *fmt, ...)
{
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    char buf[MAX_BUF_SIZE];
    char *s = (char *)buf;
    int count = vsprintf(s, fmt, args);
    while (*s)
    {
        if (*s == '\n')
            uart_putc('\r');
        uart_putc(*s++);
    }
    __builtin_va_end(args);
    return count;
}

void uart_hex(unsigned int d) {
    unsigned int n;
    int c;
    for(c=28;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_putc(n);
    }
}

char uart_get() {
    char r;
    /* wait until something is in the buffer */
    do{asm volatile("nop");}while(!(*AUX_MU_LSR&0x01));
    /* read it and return */
    r=(char)(*AUX_MU_IO);
    /* convert carriage return to newline */
    return r;
}

void enable_uart_interrupt() {
    enable_uart_tx_interrupt();
    enable_uart_rx_interrupt();
    *ENABLE_IRQS_1 |= 1 << 29;
}

void enable_uart_tx_interrupt() {
    *AUX_MU_IER |= 2;
}

void disable_uart_tx_interrupt() {
    *AUX_MU_IER &= ~(2);
}

void enable_uart_rx_interrupt() {
    *AUX_MU_IER |= 1;
}

void disable_uart_rx_interrupt() {
    *AUX_MU_IER &= ~(1);
}

void uart_tx_interrupt_handler() { // output
    // buffer empty
    if (tx_bf_r_idx == tx_bf_w_idx) {
        disable_uart_tx_interrupt();
        return;
    }

    *AUX_MU_IO = (unsigned int)tx_buffer[tx_bf_r_idx];
    tx_bf_r_idx++;
    tx_bf_r_idx %= MAX_BUF_SIZE;

    enable_uart_tx_interrupt();
}

void uart_rx_interrupt_handler() { // input
    // buffer full
    if ((rx_bf_w_idx + 1) % MAX_BUF_SIZE == rx_bf_r_idx) {
        disable_uart_rx_interrupt();
        return;
    }
    
    rx_buffer[rx_bf_w_idx] = (char)(*AUX_MU_IO);
    rx_bf_w_idx++;
    rx_bf_w_idx %= MAX_BUF_SIZE;
}

void uart_async_putc(char c) {
    // buffer full -> wait for empty space
    while ((tx_bf_w_idx + 1) % MAX_BUF_SIZE == tx_bf_r_idx) {
        enable_uart_tx_interrupt();
    }

    tx_buffer[tx_bf_w_idx] = c;
    tx_bf_w_idx++;
    tx_bf_w_idx %= MAX_BUF_SIZE;

    enable_uart_tx_interrupt();
}

char uart_async_getc() {
    // buffer empty -> wait
    while (rx_bf_r_idx == rx_bf_w_idx) {
        enable_uart_rx_interrupt();
    }

    char c = rx_buffer[rx_bf_r_idx];
    rx_bf_r_idx++;
    rx_bf_r_idx %= MAX_BUF_SIZE;

    return c;
}

int uart_async_printf(char *fmt, ...)
{
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    char buf[MAX_BUF_SIZE];
    char *s = (char *)buf;
    int count = vsprintf(s, fmt, args);
    while (*s)
    {
        if (*s == '\n')
            uart_async_putc('\r');
        uart_async_putc(*s++);
    }
    __builtin_va_end(args);
    return count;
}