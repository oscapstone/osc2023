#include "utils.h"
#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"

static char hex_char[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void uart_init() {
    unsigned int selector;

    // set the selected function for pin14,15 to mini uart
    selector = read_reg_32(GPFSEL1);

    selector &= ~(7 << 12);
    selector &= ~(7 << 15);
    selector |= (2 << 12);
    selector |= (2 << 15);

    write_reg_32(GPFSEL1, selector);


    // apply disable pull-up/pull-down setting on pin14,15
    write_reg_32(GPPUD, 0);
    delay(150);
    write_reg_32(GPPUDCLK0, (1 << 14 | 1 << 15));
    delay(150);
    write_reg_32(GPPUDCLK0, 0);

    //enable & initial mini uart
    // bcm2835 manual chapter 2
    write_reg_32(AUX_ENABLES, 1);
    write_reg_32(AUX_MU_CNTL_REG, 0);
    write_reg_32(AUX_MU_IER_REG, 0);
    write_reg_32(AUX_MU_LCR_REG, 3);
    write_reg_32(AUX_MU_MCR_REG, 0);
    write_reg_32(AUX_MU_IIR_REG, 0x06);
    write_reg_32(AUX_MU_BAUD_REG, 270);

    write_reg_32(AUX_MU_CNTL_REG, 3);
}

void uart_send(char c) {
    while(1) {
        if(read_reg_32(AUX_MU_LSR_REG) & 0x20) {
            break;
        }
    }
    write_reg_32(AUX_MU_IO_REG, c);
}

char uart_recv(void) {
    while(1) {
        if(read_reg_32(AUX_MU_LSR_REG) & 0x01) {
            break;
        }
    }
    return (read_reg_32(AUX_MU_IO_REG) & 0xff);
}

void uart_send_u64(unsigned long u64) {
    for(int cnt = 60; cnt >= 0; cnt -= 4) {
        uart_send(hex_char[(u64 >> cnt) & 0xF]);
    }
}

void uart_send_u32(unsigned int u32) {
    for(int cnt = 28; cnt >= 0; cnt -= 4) {
        uart_send(hex_char[(u32 >> cnt) & 0xF]);
    }
}

void uart_send_string(const char *c) {
    for(const char *ch = c; *ch != '\0'; ch ++) {
        uart_send(*ch);
    }
}
void uart_send_n(const char *c, unsigned long long n) {
    for(int i = 0; i < n; i ++) {
        // uart_send_u32(c[i]);
        // uart_send_string("\r\n");
        if(c[i] == '\n') {
            uart_send('\r');
        }
        uart_send(c[i]);
    }
}