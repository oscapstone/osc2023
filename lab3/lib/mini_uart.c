#include "utils.h"
#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"
#include "peripherals/exception.h"

void uart_send ( char c )
{
	if (c == '\n') uart_send('\r');

    while (1) {
		if (get32(AUX_MU_LSR_REG)&0x20) 
			break;
	}
    
	put32(AUX_MU_IO_REG,c);
}

char uart_recv ( void )
{
	while(1) {
		if (get32(AUX_MU_LSR_REG)&0x01) 
			break;
	}
	return (get32(AUX_MU_IO_REG)&0xFF);
}

void uart_send_string ( char* str )
{
	for (int i = 0; str[i] != '\0'; i++) {
		uart_send((char)str[i]);
	}
}

void uart_init ( void )
{
    unsigned int selector;

    selector = get32(GPFSEL1);
    selector &= ~(7<<12);                   // clean gpio14
    selector |= 2<<12;                      // set alt5 for gpio14
    selector &= ~(7<<15);                   // clean gpio15
    selector |= 2<<15;                      // set alt5 for gpio 15
    put32(GPFSEL1,selector);

    put32(GPPUD,0);
    delay(150);
    put32(GPPUDCLK0,(1<<14)|(1<<15));
    delay(150);
    put32(GPPUDCLK0,0);

    put32(AUX_ENABLES,1);                   //Enable mini uart (this also enables access to its registers)
    put32(AUX_MU_CNTL_REG,0);               //Disable auto flow control and disable receiver and transmitter (for now)
    put32(AUX_MU_IER_REG,1);                //Enable receive and disable transmit interrupts
    put32(AUX_MU_LCR_REG,3);                //Enable 8 bit mode
    put32(AUX_MU_MCR_REG,0);                //Set RTS line to be always high
    put32(AUX_MU_BAUD_REG,270);             //Set baud rate to 115200
    put32(AUX_MU_IIR_REG, 6);               //Interrupt identify no fifo
    put32(AUX_MU_CNTL_REG,3);               //Finally, enable transmitter and receiver

}

char async_read_buf[256];
char async_write_buf[256];
int read_s, read_e;     //read start, read end
int write_s, write_e;   //write start, write end

void enable_uart_interrupt() { put32(ENABLE_IRQS_1, (1<<29)); }
void disable_uart_interrupt() { put32(DISABLE_IRQS_1, (1<<29)); }
void set_transmit_interrupt() { put32(AUX_MU_IER_REG, 0x2); } 
void clear_transmit_interrupt() { put32(AUX_MU_IER_REG, 0x1); } 

void async_uart_handler() {
    disable_uart_interrupt();
    if (get32(AUX_MU_IIR_REG)&0x4) {
        
        char c = (char)get32(AUX_MU_IO_REG);
        async_read_buf[read_e++] = c;
        if (read_e == 256) read_e = 0;

    } else if (get32(AUX_MU_IIR_REG)&0x2) {

        //writing data
        while (get32(AUX_MU_LSR_REG)&0x20) {
            //buffer is full
            if (write_s == write_e) {
                put32(AUX_MU_IER_REG, 0x1);     //enable_read_interrupt
                break;
            }
            char c = async_write_buf[write_s++];
            put32(AUX_MU_IO_REG,c);
            //clear buffer count
            if (write_s == 256) write_s = 0;

        }

    }
    enable_uart_interrupt();
}

char async_uart_recv() {
    //gets
    while (read_s == read_e)
        asm volatile("nop");

    char c = async_read_buf[read_s++];
    if (read_s == 256) read_s = 0;

    return c;

}

void async_uart_send_string(char *str) {
    //puts
    for (int i = 0; str[i] != '\0'; i++) {
		
        if (str[i] == '\n') {
            async_write_buf[write_e++] = '\r';
            async_write_buf[write_e++] = '\n';
            continue;
        }
        async_write_buf[write_e++] = str[i];
        if (write_e == 256) write_e = 0;

	}

    set_transmit_interrupt();   //enable_write_interrupt

}

void async_uart_test() {

    enable_uart_interrupt();

    for (int i = 0; i < 10000; i++)
        asm volatile("nop");

    char buffer[256];
    unsigned int i = 0;
    while (1) {
        buffer[i] = async_uart_recv();
        if (buffer[i] == '\r') break;
        i++;
    }
    buffer[i+1] = '\n';
    buffer[i+2] = '\0';
    async_uart_send_string(buffer);
    disable_uart_interrupt();

}
