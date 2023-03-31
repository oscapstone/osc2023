#include "mini_uart.h"
#include "gpio.h"
#include "utils.h"
#include "mailbox.h"
#include "uart.h"
#include "time_interrupt.h"

char read_buf[BUFFER_MAX_SIZE];
char write_buf[BUFFER_MAX_SIZE];
int read_buf_start, read_buf_end;
int write_buf_start, write_buf_end;

void uart_init(void) 
{
    // static int uart_inited = 0;
    // if (uart_inited) return;
    /* set alternative function */
    //https://github.com/s-matyukevich/raspberry-pi-os/blob/master/docs/lesson01/rpi-os.md#mini-uart-initialization
    //register keyword ask compiler to store variable in CPU register as possible.
    register unsigned int selector = *GPFSEL1;
    selector &= ~(7<<12);                   // clean gpio14
    selector |= 2<<12;                      // set alt5 for gpio14
    selector &= ~(7<<15);                   // clean gpio15
    selector |= 2<<15;                      // set alt5 for gpio 15
    *GPFSEL1 = selector;

    /* we need neither the pull-up nor the pull-down state, because both the 14 and 15 pins are going to be connected all the time. */
    *GPPUD = 0;
    delay(150);
    *GPPUDCLK0 = ((1<<14) | (1<<15));
    delay(150);
    *GPPUDCLK0 = 0;

    /* Initializing the Mini UART */
    *AUX_ENABLES = 1;               //Enable mini uart (this also enables access to its registers)
    *AUX_MU_CNTL = 0;               //Disable auto flow control and disable receiver and transmitter (for now)
    *AUX_MU_IER = 0;                //Disable receive and transmit interrupts
    *AUX_MU_LCR = 3;                //Enable 8 bit mode
    *AUX_MU_MCR = 0;                //Set RTS line to be always high
    *AUX_MU_BAUD = 270;             //Set baud rate to 115200

    *AUX_MU_CNTL = 3;               //Finally, enable transmitter and receiver
    // uart_inited = 1;
}

char uart_read(void)
{
    char r = _uart_read();
    return r == '\r' ? '\n' : r;
}

char _uart_read(void)
{
    while (!(*AUX_MU_LSR & 0x01));
    char r = *AUX_MU_IO & 0xff;
    return r;
}

unsigned int uart_readline(char *buffer, unsigned int buffer_size)
{
    if (buffer_size == 0) return 0;
    char *ptr = buffer, *buffer_tail = buffer + buffer_size - 1;
    do {
        *ptr = uart_read();
    } while (*ptr != '\n' && (ptr++ < buffer_tail));
    *ptr = '\0';
    return (ptr - buffer);
}

unsigned long long uart_read_hex_ull() {
    unsigned long long res = 0;
    //receive 8 byte
    for (int i = 60; i >= 0; i -= 4) {
        res |= ((uart_read() - '0') << i);
    }
    return res;
}

static void _uart_write(char c)
{
    while (!(*AUX_MU_LSR & 0x20));
    *AUX_MU_IO = c;
}

void uart_write_retrace()
{
    _uart_write('\r');
}

void uart_write(char c)
{
    if (c == '\n') _uart_write('\r');
    _uart_write(c);
}

void uart_flush() {
    while (*AUX_MU_LSR & 0x01) {
        *AUX_MU_IO;
    }
}

void uart_write_string(char* str)
{
    for (int i = 0; str[i] != '\0'; i++) {
        // if (str[i] == '\r') continue;
        uart_write((char)str[i]);
    }
}

void uart_write_no(unsigned long long n) 
{
    if (n < 10) {
        uart_write('0' + n);
        return;
    } else {
        uart_write_no(n / 10);
        uart_write('0' + n % 10);
    }
}

void uart_write_no_hex(unsigned long long n)
{
    const char *hex_str = "0123456789abcdef";
    if (n < 16) {
        uart_write(hex_str[n]);
    } else {
        uart_write_no_hex(n >> 4);
        uart_write(hex_str[n & 0xf]);
    }
}

void byte_to_hex(char* buf, unsigned char byte) 
{
    const char hex_table[] = "0123456789abcdef";

    buf[0] = hex_table[byte >> 4];
    buf[1] = hex_table[byte & 0x0F];
    buf[2] = '\0';
}

void dump_hex(const void* ptr, unsigned long long size) 
{
    const unsigned char* p = (const unsigned char*)ptr;
    char buf[3];

    for (unsigned long long i = 0; i < size; i++) {
        byte_to_hex(buf, p[i]);
        uart_write_string(buf);
    }
}

unsigned int uart_read_input(char *cmd, unsigned int cmd_size)
{
    unsigned idx = 0;
    do {
        cmd[idx] = uart_read();
        if (cmd[idx] == '\0' || cmd[idx] == '\n') {
            cmd[idx] = '\0';
            uart_write_string("\n");
            return idx;
        }
        uart_write(cmd[idx]);
        idx++;
    } while (idx < cmd_size);
    cmd[idx] = '\0';
    return idx;
}

void uart_write_fraction(unsigned numerator, unsigned denominator, unsigned deg)
{
    unsigned q = numerator / denominator;
    numerator = (numerator - q * denominator) * 10;
    uart_write_no(q);
    uart_write('.');
    while (deg--) {
        q = numerator / denominator;
        numerator = (numerator - q * denominator) * 10;
        uart_write('0' + q);
    }
}
///////////////////////// async part /////////////////////

void uart_irq_handler(void *arg)
{
    //To enable mini UART’s interrupt, you need to set AUX_MU_IER_REG(0x3f215044)
    // *AUX_MU_IER = 1;
    //and the second level interrupt controller’s Enable IRQs1(0x3f00b210)’s bit29.
    //why 29?
    //https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf p113
    *ENB_IRQS1 |= (1 << 29);
    reg_t ier = (reg_t)arg;
    ier &= ~(0x3);

    if (*AUX_MU_IIR & 0x4) {
        char c = (char)(*AUX_MU_IO);
        read_buf[read_buf_end] = c;
        read_buf_end = (read_buf_end + 1) % BUFFER_MAX_SIZE;
    }
    else if (*AUX_MU_IIR & 0x2) {
        //AUX_MU_LSR&0x20是用來判斷 c 有沒有被寫進AUX_MU_IO
        while (*AUX_MU_LSR & 0x20) {
            if (write_buf_start == write_buf_end) {
                //nothing to write, disable interrupt
                ier &= ~(0x2);
                break;
            }
            char c = write_buf[write_buf_start];
            *AUX_MU_IO = c;
            write_buf_start = (write_buf_start + 1) % BUFFER_MAX_SIZE;
        }
    }
    *AUX_MU_IER = ier;
    *DISABLE_IRQS1 &= ~(1 << 29);
}

char _async_uart_read()
{
    // Enable R interrupt
    *AUX_MU_IER |= 0b1;
    while (read_buf_start == read_buf_end);
    char c = read_buf[read_buf_start];
    read_buf_start = (read_buf_start + 1) % BUFFER_MAX_SIZE;
    return c;
}

char async_uart_read()
{
    char c = _async_uart_read();
    return c == '\r' ? '\n' : c;
}

void _async_uart_write(char c)
{
    write_buf[write_buf_end] = c;
    write_buf_end = (write_buf_end + 1) % BUFFER_MAX_SIZE;
    // Enable W interrupt
    *AUX_MU_IER |= 0x2;
}

void async_uart_write(char c)
{
    if (c == '\n') _async_uart_write('\r');
    _async_uart_write(c);
}

unsigned int async_uart_readline(char *buffer, unsigned int buffer_size)
{
    if (buffer_size == 0) return 0;
    char *ptr = buffer, *buffer_tail = buffer + buffer_size - 1;
    do {
        *ptr = async_uart_read();
    } while (*ptr != '\n' && (ptr++ < buffer_tail));
    *ptr = '\0';
    return (ptr - buffer);
}

void async_uart_write_string(char* str)
{
    for (int i = 0; str[i] != '\0'; i++) {
        async_uart_write((char)str[i]);
    }
}

void test_uart_async()
{
    *ENB_IRQS1 |= (1 << 29);
    delay(15000);
    char buffer[BUFFER_MAX_SIZE];
    size_t index = 0;
    while (index < BUFFER_MAX_SIZE-1)
    {
        buffer[index] = async_uart_read();
        // uart_async_send(buffer[index]);
        if (buffer[index] == '\n')
        {
            break;
        }
        index++;
    }
    buffer[index + 1] = '\0';
    async_uart_write_string(buffer);
    *DISABLE_IRQS1 &= ~(1 << 29);
}