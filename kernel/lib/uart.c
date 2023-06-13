#include "uart.h"

char uart_tx_buffer[MAX_BUF_SIZE] = {};
unsigned int uart_tx_buffer_w_idx = 0;
unsigned int uart_tx_buffer_r_idx = 0; 
char uart_rx_buffer[MAX_BUF_SIZE] = {};
unsigned int uart_rx_buffer_w_idx = 0;
unsigned int uart_rx_buffer_r_idx = 0;

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

void enable_mini_uart_interrupt()
{
    enable_mini_uart_rx_interrupt();
    enable_mini_uart_tx_interrupt();
    *ENABLE_IRQS_1 |= 1 << 29;
}

void enable_mini_uart_rx_interrupt()
{
    *AUX_MU_IER |= 1; // rx interrupt
}

void enable_mini_uart_tx_interrupt()
{
    *AUX_MU_IER |= 2; // tx interrupt
}

void disable_mini_uart_interrupt()
{
    disable_mini_uart_rx_interrupt();
    disable_mini_uart_tx_interrupt();
    *DISABLE_IRQS_1 |= 1 << 29;
}

void disable_mini_uart_rx_interrupt() // input
{
    *AUX_MU_IER &= ~(1);
}

void disable_mini_uart_tx_interrupt() // output
{
    *AUX_MU_IER &= ~(2);
}

void uart_rx_interrupt_handler() // input
{
    uart_rx_buffer[uart_rx_buffer_w_idx++] = uart_getc();
    uart_rx_buffer_w_idx %= MAX_BUF_SIZE;

    enable_mini_uart_rx_interrupt();
}

void uart_tx_interrupt_handler() // output
{
    if (uart_tx_buffer_r_idx == uart_tx_buffer_w_idx) // Empty
    {
        disable_mini_uart_tx_interrupt();
        return;
    }

    uart_putc(uart_tx_buffer[uart_tx_buffer_r_idx++]);
    uart_tx_buffer_r_idx %= MAX_BUF_SIZE;

    enable_mini_uart_tx_interrupt();
}

void uart_async_putc(char c)
{
    while ((uart_tx_buffer_w_idx + 1) % MAX_BUF_SIZE == uart_tx_buffer_r_idx) // Full
        enable_mini_uart_tx_interrupt();

    // critical section
    disable_interrupt();
    
    uart_tx_buffer[uart_tx_buffer_w_idx++] = c;
    uart_tx_buffer_w_idx %= MAX_BUF_SIZE;

    enable_interrupt();

    enable_mini_uart_tx_interrupt();
}

char uart_async_getc()
{
    enable_mini_uart_rx_interrupt(); // Enable read interrupt to write input into rx buffer via interrupt handler
    while (uart_rx_buffer_r_idx == uart_rx_buffer_w_idx)
        enable_mini_uart_rx_interrupt();

    disable_interrupt();

    char r = uart_rx_buffer[uart_rx_buffer_r_idx++];
    uart_rx_buffer_r_idx %= MAX_BUF_SIZE;

    enable_interrupt();

    return r;
}

char *uart_async_gets(char *buf)
{
    int count;
    char c;
    char *s;
    for (s = buf, count = 0; (c = uart_async_getc()) != '\n' && count != MAX_BUF_SIZE - 1; count++)
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

void uart_puts(char *s)
{
    int i = 0;

    while (*s)
    {
        uart_putc(*s++);
        i++;
    }
    uart_putc('\r');
    uart_putc('\n');
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
