#include <uart.h>

#include <stdint.h>
#include <stddef.h>

#include <utils.h>

void uart_init(){
    //12-14, 15-17
    // get oringin val, reset gpio14-15, set alt5 0b010
    memory_write(GPFSEL1, (memory_read(GPFSEL1) & 0xfffc0fff) | (0b010010 << 12));
    //gpio page 101 1 ~ 4, 6
    memory_write(GPPUD, 0b00);
    wait_cycles(150u);
    memory_write(GPPUDCLK0, (1 << 14) | (1 << 15));
    wait_cycles(150u);
    memory_write(GPPUDCLK0, 0u);
    // Initialization 1 ~ 8
    memory_write(AUX_ENABLES, 0b001);
    memory_write(AUX_MU_CNTL_REG, 0u);
    memory_write(AUX_MU_IER_REG, 0u);
    memory_write(AUX_MU_LCR_REG, 3u);
    memory_write(AUX_MU_MCR_REG, 0u);
    memory_write(AUX_MU_BAUD_REG, 270u);
    memory_write(AUX_MU_IIR_REG, 6u);
    memory_write(AUX_MU_CNTL_REG, 3u);
}

size_t uart_read_1c(char*buf){
    while(!(memory_read(AUX_MU_LSR_REG) & 0x01));
    buf[0] = (char)(memory_read(AUX_MU_IO_REG) & 0xff);
    if(buf[0] == '\r') 
        uart_print("\r\n");
    else
        uart_write_1c(&buf[0]);
    return 1;
}

size_t uart_write_1c(char*buf){
    while(!(memory_read(AUX_MU_LSR_REG) & 0b100000));
    memory_write(AUX_MU_IO_REG, (memory_read(AUX_MU_IO_REG) & 0xffffff00) | buf[0]);
    return 1;
}

size_t uart_write(char* buf, size_t len){
    size_t writelen = 0;
    while(writelen < len){
        uart_write_1c(&buf[writelen++]);
    }
    return writelen;
}

size_t uart_readline(char* buf){
    int i = 0;
    while(uart_read_1c(&buf[i])){
        if(buf[i] == '\r'){
            buf[i] = '\0';
            break;
        }
        i++;
    }
    return i;
}

void uart_print(char* buf){
    int i=0;
    while(buf[i]){
        uart_write_1c(&buf[i]);
        i++;
    }
}

void uart_print_hex(uint64_t num, int len){
    uint64_t tmp;
    char hex[20] = {};
    uart_print("0x");
    for(int i = len - 4; i >= 0; i -= 4){
        tmp = (num >> i) & 0xF;
        // 0-9 a-f
        hex[(len / 4 - 1) - i / 4] = (char) tmp + (tmp > 9 ? 'a' - 10 : '0');
    }
    uart_print(hex);
}