#include "utils.h"
#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"

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
    unsigned int result = 0;

    for (int i = 0; i < 4; ++i) {
        unsigned char c = uart_recv();
        result *= 10;
		result += c - '0';
    }

    return result;
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