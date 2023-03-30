#include "utils.h"
#include "peripherals/mini_uart.h"
#include "peripherals/gpio.h"
#include "lock.h"
#include "interrupt.h"
#include "mem.h"
#include "event.h"

#define QUEUE_FULL 1
#define QUEUE_EMPTY 2
#define QUEUE_SUCCESS 0


static char hex_char[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
static int uart_func;

static char rx_copy_buf[256];

struct k_event uart_recv_event;

extern int demo_state;

struct Uart_MSG_Queue* Uart_MSG_Queue_init() {
    // init Uart MSG Queue
    struct Uart_MSG_Queue *ret = (struct Uart_MSG_Queue*)simple_malloc(sizeof(struct Uart_MSG_Queue));
    ret->begin = 0;
    ret->end = 0;
    ret->size = 0;
    memset(ret->buf, 0, UART_QUEUE_SIZE);
    return ret;
}
void enable_transmit_interrupt() {
    // enable transmit interrupt
    write_reg_32(AUX_MU_IER_REG, 2);
}
void disable_transmit_interrupt() {
    // disable transmit interrupt
    write_reg_32(AUX_MU_IER_REG, 1);
}



unsigned int uart_pop(struct Uart_MSG_Queue *que, unsigned int *res) {
    // this operation of pop and push msg queue need to be atomic respect to uart transmission
    if(que->size == 0) {
        enable_aux_interrupt();
        return QUEUE_EMPTY;
    }
    que->size -= 1;
    *res = que->buf[que->begin];
    que->begin += 1;
    if(que->begin == UART_QUEUE_SIZE) {
        que->begin = 0;
    }
    return QUEUE_SUCCESS;
}

unsigned int uart_push(struct Uart_MSG_Queue *que, char val) {
    // this operation of pop and push msg queue need to be atomic
    if(que->size == UART_QUEUE_SIZE) {
        return QUEUE_FULL;
    }
    que->size += 1;
    que->buf[que->end] = val;
    que->end += 1;
    if(que->end == UART_QUEUE_SIZE) {
        que->end = 0;
    }
    return QUEUE_SUCCESS;
}

static struct Uart_MSG_Queue *rxbuf;
static struct Uart_MSG_Queue *txbuf;

// static uint16_t* uart_func_sw;

void _putchar(char c) {
    while(1) {
        if(read_reg_32(AUX_MU_LSR_REG) & 0x20) {
            break;
        }
    }
    write_reg_32(AUX_MU_IO_REG, c);
}

char _getchar() {
    while(1) {
        if(read_reg_32(AUX_MU_LSR_REG) & 0x01) {
            break;
        }
    }
    return (read_reg_32(AUX_MU_IO_REG) & 0xff);
}

// not very good implement
// should implement the same way as interrupt_disable_save
void enable_aux_interrupt() {
    write_reg_32(ENABLE_IRQ1, (1 << 29));
}
void disable_aux_interrupt() {
    write_reg_32(DISABLE_IRQ1, (1 << 29));
}

void uart_rx_to_buf(void* ptr, uint32_t sz) {
    // TODO reconstruct demo codes
    if(demo_state) {
        uart_send(*(char*)ptr);
    }
    for(int i = 0; i < sz; i ++) {
        uart_push(rxbuf, *(char*)ptr);
    }
}

void uart_init() {
    unsigned int selector;

    // uart_func_sw = (uint16_t*)simple_malloc(sizeof(uart_func_sw));
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
    // enable recv interrupt
    write_reg_32(AUX_MU_IER_REG, 1);
    write_reg_32(AUX_MU_LCR_REG, 3);
    write_reg_32(AUX_MU_MCR_REG, 0);
    write_reg_32(AUX_MU_IIR_REG, 0x06);
    write_reg_32(AUX_MU_BAUD_REG, 270);

    write_reg_32(AUX_MU_CNTL_REG, 3);
    // init msg queue
    rxbuf = Uart_MSG_Queue_init();
    txbuf = Uart_MSG_Queue_init();
    uart_func = UART_ASYNC;
    k_event_init(&uart_recv_event, &uart_rx_to_buf);
    // uart_func = UART_DEFAULT;
}



void uart_send(char c) {
    if(uart_func == UART_ASYNC) {
        async_uart_send(c);
        return;
    } else {
        _putchar(c);
    }
}

char uart_recv(void) {
    if(uart_func == UART_ASYNC) {
        return async_uart_recv();
    } else {
        return _getchar();
    }
}

char async_uart_recv(void) {
    // first disable aux interrupt so that rxbuf won't race
    unsigned int val;
    disable_aux_interrupt();
    int err = uart_pop(rxbuf, &val);
    enable_aux_interrupt();
    if(err == QUEUE_EMPTY) {
        return 255;
    }
    return (val & 0xff);
}

void async_uart_send(char c) {
    // first disable aux interrupt so that txbuf won't race
    disable_aux_interrupt();
    uart_push(txbuf, c);
    enable_transmit_interrupt();
    enable_aux_interrupt();
}

void async_uart_send_string(const char *buf) {
    // first disable aux interrupt so that txbuf won't race
    disable_aux_interrupt();
    for(const char *ch = buf; *ch != '\0'; ch ++) {
        uart_push(txbuf, *ch);
    }
    enable_transmit_interrupt();
    enable_aux_interrupt();
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
    if(uart_func == UART_ASYNC) {
        async_uart_send_string(c);
    } else {
        for(const char *ch = c; *ch != '\0'; ch ++) {
            uart_send(*ch);
        }
    }
}
void uart_send_n(const char *c, unsigned long long n) {
    for(int i = 0; i < n; i ++) {
        if(c[i] == '\n') {
            uart_send('\r');
        }
        uart_send(c[i]);
    }
}
void uart_send_dec(unsigned long dec) {
    char buf[20];
    int idx = 0;
    while(dec) {
        buf[idx ++] = (dec % 10) + '0';
        dec /= 10;
    }
    for(int i = idx - 1; i >= 0; i --) {
        uart_send(buf[i]);
    }
}
void uart_int_trans_handler() {
    unsigned int val;
    int err;
    while(read_reg_32(AUX_MU_LSR_REG) & 0x20) {
        err = uart_pop(txbuf, &val);
        if(err == 0) {
            write_reg_32(AUX_MU_IO_REG, val % 0xff);
        } else {
            disable_transmit_interrupt();
            break;
        }
    }
}

void uart_int_recv_handler() {
    int idx = 0;
    unsigned int val;
    while(read_reg_32(AUX_MU_LSR_REG) & 0x01) {
        val = read_reg_32(AUX_MU_IO_REG);
        rx_copy_buf[idx ++] = (val & 0xff);
    }
    k_event_submit(&uart_recv_event, rx_copy_buf, idx, 200);
}
void uart_switch_func(unsigned int func) {
    uart_func = func;
}