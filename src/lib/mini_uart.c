#include <utils.h>
#include <mini_uart.h>
#include <BCM.h>
#include <type.h>
#include <stdarg.h>
#include <irq.h>

#define BUFSIZE 0x100

static void uart_irq_fini(void);

static int uart_sync_mode;

typedef char (*recvfp)(void);
typedef void (*sendfp)(char);

recvfp uart_recv_fp;
sendfp uart_send_fp;

static char w_buffer[BUFSIZE];
static char r_buffer[BUFSIZE];
static int w_head, w_tail, r_head, r_tail;

static inline void enable_R_interrupt(){
    uint32 ier = get32(PA2VA(AUX_MU_IER_REG));
    ier |= 0x01;
    put32(PA2VA(AUX_MU_IER_REG), ier);
}

static inline void enable_W_interrupt(){
    uint32 ier = get32(PA2VA(AUX_MU_IER_REG));
    ier |= 0x02;
    put32(PA2VA(AUX_MU_IER_REG), ier);
}

static inline void disable_RW_interrupt(){
    put32(PA2VA(AUX_MU_IER_REG), 0);
}

static inline void enable_RW_interrupt(){
    put32(PA2VA(AUX_MU_IER_REG), 3);
}

static char uart_async_recv(void){
    // Enable R interrupt
    enable_R_interrupt();
    while(1){
        if(r_head != r_tail)
            break;
    }

    char tmp = r_buffer[r_head];
    r_head = (r_head + 1) % BUFSIZE;

    return tmp;
}

static void uart_async_send(char c)
{
    while(1){
        if(w_head != (w_tail + 1) % BUFSIZE)
            break;
    }

    w_buffer[w_tail] = c;
    w_tail = (w_tail + 1) % BUFSIZE;

    // Enable W interrupt
    enable_W_interrupt();
}

static char uart_sync_recv (void){
	while(1) {
		if(get32(PA2VA(AUX_MU_LSR_REG))&0x01) 
			break;
	}
	return(get32(PA2VA(AUX_MU_IO_REG))&0xFF);
}

static void uart_sync_send (char c){
	while(1) {
		if(get32(PA2VA(AUX_MU_LSR_REG))&0x20) // hang until can read
			break;
	}
	put32(PA2VA(AUX_MU_IO_REG),c);
}

static void uart_send_string(sendfp _send_fp, char *str){
	for (int i = 0; str[i] != '\0'; i ++)
		(_send_fp)(str[i]);
}

static void uart_send_num(sendfp _send_fp, int64 num, int base, int type)
{
    static const char digits[16] = "0123456789ABCDEF";
    char tmp[66];
    int i;

    if (type & 1) {
        if (num < 0) {
            (_send_fp)('-');
        }
    }

    i = 0;

    if (num == 0) {
        tmp[i++] = '0';
    } else {
        while (num != 0) {
            uint8 r = (uint64)num % base;
            num = (uint64)num / base;
            tmp[i++] = digits[r];
        }
    }

    while (--i >= 0) {
        (_send_fp)(tmp[i]);
    }
}

static void _uart_printf(sendfp _send_fp, const char *fmt, va_list args){

    const char *s;
    char c;
    uint32 num;
    char width;

    for (; *fmt; ++fmt) {
        if (*fmt != '%') {
            (_send_fp)(*fmt);
            continue;
        }

        ++fmt;

        width = 0;
        if (fmt[0] == 'l' && fmt[1] == 'l') {
            width = 1;
            fmt += 2;
        }

        switch (*fmt) {
        case 'c':
            c = va_arg(args, uint32) & 0xff;
            (_send_fp)(c);
            continue;
        case 'd':
            if(width)
                num = va_arg(args, int64);
            else
                num = va_arg(args, int32);
            uart_send_num(_send_fp, num, 10, 1);
            continue;
        case 's':
            s = va_arg(args, char *);
            uart_send_string(_send_fp,(char*)s);
            continue;
        case 'x':
            if (width) 
                num = va_arg(args, uint64);
            else
                num = va_arg(args, uint32);
            uart_send_num(_send_fp, num, 16, 0);
            continue;
        }
    }
}

static void uart_irq_handler(void *_){
    uint32 iir = get32(PA2VA(AUX_MU_IIR_REG));

    // Transmit holding register empty
    if(iir & 0x02){
        /* if head equals to tail ==> means the write buffer is empty. */

        /* if head not equals to tail ==> means write buffer is not empty
           Then we need to write the buffer to IO register and move the buffer pointer backward*/
        if(w_head != w_tail){
            put32(PA2VA(AUX_MU_IO_REG), w_buffer[w_head]);
            w_head = (w_head+1)%BUFSIZE;
        }
    }

    // Receiver holds valid byte
    else if(iir & 0x04){
        // if head not equals to tail+1 ==> means read buffer still have some place
        if(r_head != (r_tail+1)%BUFSIZE){
            r_buffer[r_tail] = get32(PA2VA(AUX_MU_IO_REG)) & 0xff;
            r_tail = (r_tail+1)%BUFSIZE;
        }
    }
}

static void uart_irq_fini(){
    uint32 ier = get32(PA2VA(AUX_MU_IER_REG));
    ier &= ~(0x03);

    if (r_head != (r_tail + 1) % BUFSIZE) {
        ier = ier | 0x01;
    }

    if (w_head != w_tail) {
        ier = ier | 0x02;
    }

    put32(PA2VA(AUX_MU_IER_REG), ier);
}

char uart_recv(void){
    return (uart_recv_fp)();
}

void uart_recvn(char *buff, int n){
    while(n--)
        *buff++ = (uart_recv_fp)();
}

void uart_send(char c){
    (uart_send_fp)(c);
}

void uart_printf(const char *fmt, ...){
    va_list args;
    va_start(args, fmt);

    _uart_printf(uart_send_fp, fmt, args);

    va_end(args);
}

void uart_sync_printf(const char *fmt, ...){
    va_list args;
    va_start(args, fmt);

    _uart_printf(uart_sync_send, fmt, args);

    va_end(args);
}

void uart_sync_vprintf(const char *fmt, va_list args){
    _uart_printf(uart_sync_send, fmt, args);
}

int uart_recv_line(char *buf, int maxline){
	int cnt = 0;
	maxline--;

	while(maxline){
		char c = uart_recv();
		if(c == '\r')
			break;
		uart_send(c);
		*buf = c;
		buf++;
		cnt++;
		maxline--;
	}

	*buf = 0;
	return cnt;
}

uint32 uart_recv_uint(void){
	char buf[4];
    
    for (int i = 0; i < 4; ++i) {
        buf[i] = uart_recv();
    }

    return *((uint32*)buf);
}

void uart_sendn(const char *str, int n){
    while (n--) 
        uart_send(*str++);
}

void uart_init (void)
{
	unsigned int selector;

	selector = get32(PA2VA(GPFSEL1));
	selector &= ~(7<<12);                   // clean gpio14 (rx)
	selector |= 2<<12;                      // set alt5 for gpio14
	selector &= ~(7<<15);                   // clean gpio15 (tx)
	selector |= 2<<15;                      // set alt5 for gpio15
	put32(PA2VA(GPFSEL1),selector);

	put32(PA2VA(GPPUD),0);                         //Disable pull up/down, floating input pin
	delay(150);
	put32(PA2VA(GPPUDCLK0),(1<<14)|(1<<15));
	delay(150);
	put32(PA2VA(GPPUDCLK0),0);

	put32(PA2VA(AUX_ENABLES),1);                   //Enable mini uart (this also enables access to its registers)
	put32(PA2VA(AUX_MU_CNTL_REG),0);               //Disable auto flow control and disable receiver and transmitter (for now)
	put32(PA2VA(AUX_MU_IER_REG),0);                //Disable receive and transmit interrupts
	put32(PA2VA(AUX_MU_LCR_REG),3);                //Enable 8 bit mode
	put32(PA2VA(AUX_MU_MCR_REG),0);                //Set RTS line to be always high
	put32(PA2VA(AUX_MU_BAUD_REG),270);             //Set baud rate to 115200
	put32(PA2VA(AUX_MU_IIR_REG), 6);               //Clear the Rx/Tx FIFO
	put32(PA2VA(AUX_MU_CNTL_REG),3);               //Finally, enable transmitter and receiver

    // UART start from synchronous mode
    uart_sync_mode = 0;
    uart_recv_fp = uart_sync_recv;
    uart_send_fp = uart_sync_send;
}
int uart_irq_add(){
    uint32 iir = get32(PA2VA(AUX_MU_IIR_REG));
    // No interrupt
    if(iir & 0x01)
        return 0;

    disable_RW_interrupt();
    if(irq_add_task(uart_irq_handler, NULL,uart_irq_fini, UART_PRIO))
        enable_RW_interrupt();

    return 1;
}

int uart_switch_mode(void){
    uart_sync_mode = !uart_sync_mode;
    uint32 ier = get32(PA2VA(AUX_MU_IER_REG));
    // set bit 0 and bit 1 to zero
    ier &= ~(0x03);

    if(uart_sync_mode == 0){
        // synchronous mode
        uart_recv_fp = uart_sync_recv;
        uart_send_fp = uart_sync_send;

        // disable interrupt;
        put32(PA2VA(AUX_MU_IER_REG), ier);
    }
    else{
        // asynchronous mode
        uart_recv_fp = uart_async_recv;
        uart_send_fp = uart_async_send;

        // clear Rx/Tx FIFO
        put32(PA2VA(AUX_MU_IIR_REG), 6);


        // enable receive interrupt
        ier |= 0x01;
        put32(PA2VA(AUX_MU_IER_REG), ier);
    }

    return uart_sync_mode;
}