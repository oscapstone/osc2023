#include "utils.h"
#include "mini_uart.h"
#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"
#include "exception.h"


char input_buf[1024];
char output_buf[1024];
unsigned int input_head = 0, input_tail = 0;
unsigned int output_head = 0, output_tail = 0;

void uart_send ( char c )
{
	while(1) {
		if(get32(AUX_MU_LSR_REG)&0x20) // bit 5, empty
			break;
	}
	put32(AUX_MU_IO_REG,c);
}

char uart_recv ( void )
{
	while(1) {
		if(get32(AUX_MU_LSR_REG)&0x01) // bit 0, data ready
			break;
	}
	return get32(AUX_MU_IO_REG)&0xFF;
}

unsigned int uart_recv_int ( void )
{
    unsigned int num = 0;
    unsigned char c;

    while ((c = uart_recv()) && c >= '0' && c <= '9') {
        num = 10 * num + (c - '0');
    }

    return num;
}

void uart_puts(char* str)
{
	for (int i = 0; str[i] != '\0'; i ++) {
		uart_send((char)str[i]);
	}
}

void uart_gets(char* str)
{
	char c;
	int size = 0;
	while(1){
        c = uart_recv();

        if(c == '\n' || c == '\r'){
            uart_puts("\r\n");
            str[size] = '\0';
            break;
        }
        else if(c == BACKSPACE || c == DEL){
            if(size > 0){
                uart_send('\b');
                uart_send(' ');
                uart_send('\b');
                str[--size] = '\0';
            }
        }
        else if((c > 16 && c < 32) || c > 127){
            continue;
        }
        else{
            str[size++] = c;
            uart_send(c);
        }
    }
}

void uart_hex(unsigned int d) {
    unsigned int n;
    int c;
    for(c=28;c>=0;c-=4) {
        // get highest tetrad
        n=(d>>c)&0xF;
        // 0-9 => '0'-'9', 10-15 => 'A'-'F'
        n+=n>9?0x37:0x30;
        uart_send(n);
    }
}

void uart_dec(unsigned int d) {
    int c = 1;
    unsigned int n = d, div = 1;
    // determine divisor
    while (n >= 10) {
        n /= 10;
        div *= 10;
        c++;
    }
    // output digits in reverse order
    while (c--) {
        uart_send((char)('0' + d / div));
        d %= div;
        div /= 10;
    }
}

void uart_init ( void )
{
	unsigned int selector;

	// GPIO alternative function selection
	selector = get32(GPFSEL1);				// GPFSEL1 register is used to control alternative functions for pins 10-19
	selector &= ~(7<<12);                   // clean gpio14
	selector |= 2<<12;                      // set alt5 for gpio14
	selector &= ~(7<<15);                   // clean gpio15
	selector |= 2<<15;                      // set alt5 for gpio15
	put32(GPFSEL1,selector);

	// Disable GPIO pull up/down
	put32(GPPUD,0);							
	delay(150);
	put32(GPPUDCLK0,(1<<14)|(1<<15));		// GPIO Pull-up/down Clock Registers
	delay(150);
	put32(GPPUDCLK0,0);


	// Initialization register
	put32(AUX_ENABLES,1);                   // Enable mini uart (this also enables access to its registers)
	put32(AUX_MU_CNTL_REG,0);               // Disable auto flow control and disable receiver and transmitter (for now)
	put32(AUX_MU_IER_REG,0);                // Disable receive and transmit interrupts
	put32(AUX_MU_LCR_REG,3);                // Enable 8 bit mode
	put32(AUX_MU_MCR_REG,0);                // Set RTS line to be always high
	put32(AUX_MU_BAUD_REG,270);             // Set baud rate to 115200
	put32(AUX_MU_IIR_REG,6);             	// No FIFO

	put32(AUX_MU_CNTL_REG,3);               // Finally, enable transmitter and receiver
}

void uart_ptr(void* ptr) {
    const char hex_table[] = "0123456789ABCDEF";
    unsigned long long val = (unsigned long long) ptr;
	char buffer[256];
    int i;
    for (i = 0; i < sizeof(void*); ++i) {
        buffer[sizeof(void*) - 1 - i] = hex_table[val & 0xf];
        val >>= 4;
    }
    buffer[sizeof(void*)] = '\0';
	uart_puts(buffer);
}

// lab3 async uart

void enable_uart_interrupt() {
    enable_read_interrupt();

    *ENABLE_IRQS_1 |= (1 << 29); //interrupt controller’s Enable IRQs1(0x3f00b210)’s bit29.
                                //BCM2837 SPEC P112 table
                                //0x210 enable IRQs1
}

void disable_uart_interrupt() {
    disable_write_interrupt();

    *DISABLE_IRQS_1 |= (1 << 29);
                                //BCM2837 SPEC P112 table
                                //0x21C disable IRQs1
}

void enable_read_interrupt() {

    *((volatile unsigned int*)AUX_MU_IER_REG) |= 0x01;
}

void disable_read_interrupt() {

    *((volatile unsigned int*)AUX_MU_IER_REG) &= ~0x01;
}

void enable_write_interrupt() {

    *((volatile unsigned int*)AUX_MU_IER_REG) |= 0x02;
}

void disable_write_interrupt() {

    *((volatile unsigned int*)AUX_MU_IER_REG) &= ~0x02;
}

void async_uart_handler() {

    disable_uart_interrupt();
    //The AUX_MU_IIR_REG register shows the interrupt status. 
    if (*((volatile unsigned int*)AUX_MU_IIR_REG) & 0x04) { // 100 //read mode   Receiver holds valid byte 
    
        char c = *((volatile unsigned int*)AUX_MU_IO_REG)&0xFF;
        input_buf[input_tail++] = c;
        if(input_tail==1024)input_tail=0;

    
    } else if (*((volatile unsigned int*)AUX_MU_IIR_REG) & 0x02) { //010  //send mode // check write enabled // Transmit holding register empty

        
        while (*((volatile unsigned int*)AUX_MU_LSR_REG) & 0x20) { //0010 0000 //Both bits [7:6] always read as 1 as the FIFOs are always enabled 
            if (output_head == output_tail) {     
                enable_read_interrupt(); 
                break;
            }

            char c = output_buf[output_head];
            output_head++;
            *((volatile unsigned int*)AUX_MU_IO_REG) = c;
            if(output_head==1024)output_head=0;
        }
    }

    enable_uart_interrupt();
}

unsigned int async_uart_gets(char *input, unsigned int size) {

    int len=0;
    for (len = 0; len < size - 1; len++) {
        while (input_head == input_tail) asm volatile("nop \r\n");
        

        if (input_buf[input_head] == '\r' || input_buf[input_head] == '\n') {
            input_head++;
            if(input_head >= 1024 -1 ) input_head -= 1024;
            break;
        }

        input[len] = input_buf[input_head];
        input_head++;
        if(input_head==1024)input_head=0;
    }

    input[len] = '\0';

    return len;
}



void async_uart_puts(const char *s) {
    
    while (*s != '\0') {
        output_buf[output_tail++] = *s++;
        if(output_tail==1024)output_tail=0;
    }
    enable_write_interrupt();
}