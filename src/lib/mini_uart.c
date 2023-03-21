#include <utils.h>
#include <mini_uart.h>
#include <BCM.h>
#include <type.h>
#include <stdarg.h>

void uart_send (char c){
	while(1) {
		if(get32(AUX_MU_LSR_REG)&0x20) // hang until can read
			break;
	}
	put32(AUX_MU_IO_REG,c);
}

void uart_send_string(char* str){
	for (int i = 0; str[i] != '\0'; i ++)
		uart_send((char)str[i]);
}

void uart_send_hex(unsigned int d) {
    unsigned int n;
    int c;
	uart_send_string("0x");
    for(c=28;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_send(n);
    }
}

char uart_recv (void){
	while(1) {
		if(get32(AUX_MU_LSR_REG)&0x01) 
			break;
	}
	return(get32(AUX_MU_IO_REG)&0xFF);
}

int uart_recv_line(char* buf, int maxline){
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

static void uart_send_num(int32 num, int base, int type)
{
    static const char digits[16] = "0123456789ABCDEF";
    char tmp[66];
    int i;

    if (type & 1) {
        if (num < 0) {
            uart_send('-');
        }
    }

    i = 0;

    if (num == 0) {
        tmp[i++] = '0';
    } else {
        while (num != 0) {
            uint8 r = (uint32)num % base;
            num = (uint32)num / base;
            tmp[i++] = digits[r];
        }
    }

    while (--i >= 0) {
        uart_send(tmp[i]);
    }
}

void uart_printf(char *fmt, ...){

    const char *s;
    char c;
    uint32 num;
    char width;   

    va_list args;
    va_start(args, fmt);

    for (; *fmt; ++fmt) {
        if (*fmt != '%') {
            uart_send(*fmt);
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
            uart_send(c);
            continue;
        case 'd':
            if(width)
                num = va_arg(args, int64);
            else
                num = va_arg(args, int32);
            uart_send_num(num, 10, 1);
            continue;
        case 's':
            s = va_arg(args, char *);
            uart_send_string((char*)s);
            continue;
        case 'x':
            if (width) 
                num = va_arg(args, uint64);
            else
                num = va_arg(args, uint32);
            uart_send_num(num, 16, 0);
            continue;
        }
    }
}

void uart_sendn(char *str, int n){
    while (n--) 
        uart_send(*str++);
}

void uart_init (void)
{
	unsigned int selector;

	selector = get32(GPFSEL1);
	selector &= ~(7<<12);                   // clean gpio14 (rx)
	selector |= 2<<12;                      // set alt5 for gpio14
	selector &= ~(7<<15);                   // clean gpio15 (tx)
	selector |= 2<<15;                      // set alt5 for gpio15
	put32(GPFSEL1,selector);

	put32(GPPUD,0);                         //Disable pull up/down, floating input pin
	delay(150);
	put32(GPPUDCLK0,(1<<14)|(1<<15));
	delay(150);
	put32(GPPUDCLK0,0);

	put32(AUX_ENABLES,1);                   //Enable mini uart (this also enables access to its registers)
	put32(AUX_MU_CNTL_REG,0);               //Disable auto flow control and disable receiver and transmitter (for now)
	put32(AUX_MU_IER_REG,0);                //Disable receive and transmit interrupts
	put32(AUX_MU_LCR_REG,3);                //Enable 8 bit mode
	put32(AUX_MU_MCR_REG,0);                //Set RTS line to be always high
	put32(AUX_MU_BAUD_REG,270);             //Set baud rate to 115200
	put32(AUX_MU_IIR_REG, 6);               //Clear the Rx/Tx FIFO
	put32(AUX_MU_CNTL_REG,3);               //Finally, enable transmitter and receiver
}