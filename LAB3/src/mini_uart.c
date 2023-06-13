#include "peripheral/mini_uart.h"
#include "peripheral/gpio.h"
#include "sprintf.h"
#include "utils_c.h"
#include <stddef.h>
#include <stdarg.h>
#include "mini_uart.h"

#define ENABLE_RECEIVE_INTERRUPTS_BIT (1 << 0)
#define ENABLE_TRANSMIT_INTERRUPTS_BIT (1 << 1)
#define AUX_INT_BIT (1 << 29)

#define BUFFER_MAX_SIZE 256u

char read_buf[BUFFER_MAX_SIZE];
char write_buf[BUFFER_MAX_SIZE];
int read_buf_start, read_buf_end;
int write_buf_start, write_buf_end;



void delay(unsigned int clock)
{
    while (clock--)
    {
        asm volatile("nop");
    }
}

// To enable mini UART’s interrupt, 
// you need to set AUX_MU_IER_REG(0x3f215044) and 
// the second level interrupt controller’s Enable IRQs1(0x3f00b210)’s bit29.
void enable_uart_interrupt() { *ENB_IRQS1 = AUX_IRQ; }

void disable_uart_interrupt() { *DISABLE_IRQS1 = AUX_IRQ; }

void set_transmit_interrupt() { *AUX_MU_IER_REG |= 0x2; }

void clear_transmit_interrupt() { *AUX_MU_IER_REG &= ~(0x2); }

void uart_init()
{
    unsigned int selector;

    selector = *GPFSEL1;
    selector &= ~(7u << 12); // clean gpio14
    selector |= 2u << 12;    // set alt5 for gpio14
    selector &= ~(7u << 15); // clean gpio15
    selector |= 2u << 15;    // set alt5 for gpio 15
    *GPFSEL1 = selector;

    *GPPUD = 0;  // set the required control signal (i.e. Pull-up or Pull-Down )
    delay(150u); // provides the required set-up time for the control signal
    *GPPUDCLK0 = (1u << 14) | (1u << 15);
    delay(150u);
    *GPPUDCLK0 = 0u;
    *AUX_ENABLE = 1u;        // Enable mini uart (this also enables access to its registers)
    *AUX_MU_CNTL_REG = 0u;   // Disable auto flow control and disable receiver and transmitter (for now)
    *AUX_MU_IER_REG = 1u;    // Enable receive
    *AUX_MU_LCR_REG = 3u;    // Enable 8 bit mode
    *AUX_MU_MCR_REG = 0u;    // Set RTS line to be always high
    *AUX_MU_BAUD_REG = 270u; // Set baud rate to 115200
    *AUX_MU_IIR_REG = 6;

    *AUX_MU_CNTL_REG = 3; // Finally, enable transmitter and receiver

    read_buf_start = read_buf_end = 0;
    write_buf_start = write_buf_end = 0;
    // enable_uart_interrupt();
}

void uart_send(const char c)
{
    /*
    bit_5 == 1 -> writable
    0x20 = 0000 0000 0010 0000
    ref BCM2837-ARM-Peripherals p5
    */
    if (c == '\n')
    {
        uart_send('\r');
    }
    while (!(*(AUX_MU_LSR_REG)&0x20))
    {
    }
    *AUX_MU_IO_REG = c;
}
char uart_recv()
{
    /*
    bit_0 == 1 -> readable
    0x01 = 0000 0000 0000 0001
    ref BCM2837-ARM-Peripherals p5
    */
    while (!(*(AUX_MU_LSR_REG)&0x01))
    {
    }
    char temp = *(AUX_MU_IO_REG)&0xFF;
    return temp == '\r' ? '\n' : temp;
}

char uart_recv_raw()
{
    /*
    bit_0 == 1 -> readable
    0x01 = 0000 0000 0000 0001
    ref BCM2837-ARM-Peripherals p5
    */
    while (!(*(AUX_MU_LSR_REG)&0x01))
    {
    }
    char temp = *(AUX_MU_IO_REG)&0xFF;
    return temp;
}

void uart_send_string(const char *str)
{
    while (*str)
    {
        uart_send(*str++);
    }
}

void uart_send_int(int num, int newline)
{
    char str[256];
    utils_int2str_dec(num, str);
    uart_send_string(str);
    if (newline)
    {
        uart_send_string("\n");
    }
}
void uart_send_uint(unsigned int num, int newline)
{
    char str[256];
    utils_uint2str_dec(num, str);
    uart_send_string(str);
    if (newline)
    {
        uart_send_string("\n");
    }
}

void uart_hex(unsigned int d)
{
    unsigned int n;
    int c;
    uart_send_string("0x");
    for (c = 28; c >= 0; c -= 4)
    {
        n = (d >> c) & 0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n += n > 9 ? 0x57 : 0x30;
        uart_send(n);
    }
}
void uart_dec(unsigned int num)
{
    if (num == 0)
        uart_send('0');
    else
    {
        if (num >= 10)
            uart_dec(num / 10);
        uart_send(num % 10 + '0');
    }
}

unsigned int uart_printf(char *fmt, ...)
{
    char dst[100];
    // __builtin_va_start(args, fmt): "..." is pointed by args
    // __builtin_va_arg(args,int): ret=(int)*args;args++;return ret;
    __builtin_va_list args;
    __builtin_va_start(args, fmt);
    unsigned int ret = vsprintf(dst, fmt, args);
    uart_send_string(dst);
    return ret;
}

/*
    Asynchronous Read and Write
    async part 要了解為何需要buffer,因為要緩存資料,稍後再處理
*/

void uart_handler(void *arg)
{
    Reg *ier=(Reg *)arg;   // 把 void 指標轉換成 Reg 指標，arg 代表外部函數呼叫時傳入的參數
    disable_uart_interrupt();   // 關閉 UART 中斷，防止中斷重複觸發
    int RX = (*AUX_MU_IIR_REG & 0x4);   // 檢查 RX 中斷是否觸發
    int TX = (*AUX_MU_IIR_REG & 0x2);   // 檢查 TX 中斷是否觸發
    if (RX)   // 如果 RX 中斷觸發
    {
        char c = (char)(*AUX_MU_IO_REG);   // 讀取接收緩衝區中的資料
        read_buf[read_buf_end++] = c;   // 把資料寫入讀取緩衝區
        if (read_buf_end == BUFFER_MAX_SIZE)   // 如果緩衝區已滿
            read_buf_end = 0;   // 從緩衝區的起始位置開始寫入
    }
    else if (TX)   // 如果 TX 中斷觸發
    {
        while (*AUX_MU_LSR_REG & 0x20)   // 當 TX FIFO 可用寫入時
        {
            if (write_buf_start == write_buf_end)   // 如果寫入緩衝區和讀取緩衝區的位置相同
            {
                *ier&=~(0x2);   // 清除 TX 中斷，停止繼續寫入
                break;   // 跳出迴圈
            }
            if (write_buf_start == BUFFER_MAX_SIZE)   // 如果緩衝區已滿
                write_buf_start = 0;   // 從緩衝區的起始位置開始寫入
            char c = write_buf[write_buf_start++];   // 從寫入緩衝區中讀取資料
            *AUX_MU_IO_REG = c;   // 把資料寫入 UART 介面
            // if (write_buf_start == BUFFER_MAX_SIZE)
            //     write_buf_start = 0;
        }
    }
    *AUX_MU_IER_REG |= *ier;   // 設定 UART 中斷
    enable_uart_interrupt();   // 開啟 UART 中斷，允許中斷繼續觸發
}


char uart_async_recv()
{
    if (read_buf_start == BUFFER_MAX_SIZE)
        read_buf_start = 0;
    // wait until there are new data
    *AUX_MU_IER_REG |= (0x1);
    while (read_buf_start == read_buf_end)
    {
        // 这行指令的目的是在等待数据的时候暂停 CPU 的执行，防止出现过多的 CPU 占用和资源浪费，同时也能保证 CPU 正常执行其他任务。
        asm volatile("nop");
    }
    char c = read_buf[read_buf_start++];
    // if (read_buf_start == BUFFER_MAX_SIZE)
    //     read_buf_start = 0;
    return c == '\r' ? '\n' : c;
}

void uart_async_send_string(char *str)
{

    for (int i = 0; str[i]; i++)
    {
        if (write_buf_end == BUFFER_MAX_SIZE)
            write_buf_end = 0;
        if (str[i] == '\n')
        {
            write_buf[write_buf_end++] = '\r';
            write_buf[write_buf_end++] = '\n';
            // if (write_buf_end == BUFFER_MAX_SIZE)
            //     write_buf_end = 0;
            continue;
        }
        write_buf[write_buf_end++] = str[i];
        // if (write_buf_end == BUFFER_MAX_SIZE)
        //     write_buf_end = 0;
    }
    set_transmit_interrupt();
}

void uart_async_send(char c)
{
    if (write_buf_end == BUFFER_MAX_SIZE)
        write_buf_end = 0;
    if (c == '\n')
    {
        write_buf[write_buf_end++] = '\r';
        write_buf[write_buf_end++] = '\n';
        set_transmit_interrupt();
        return;
    }
    write_buf[write_buf_end++] = c;
    // if (write_buf_end == BUFFER_MAX_SIZE)
    //     write_buf_end = 0;
    set_transmit_interrupt();
}

void test_uart_async()
{
    enable_uart_interrupt();
    delay(15000);
    char buffer[BUFFER_MAX_SIZE];
    size_t index = 0;
    while (1)
    {
        buffer[index] = uart_async_recv();
        // uart_async_send(buffer[index]);
        if (buffer[index] == '\n')
        {
            break;
        }
        index++;
    }
    buffer[index + 1] = '\0';
    uart_async_send_string(buffer);
    disable_uart_interrupt();
}
