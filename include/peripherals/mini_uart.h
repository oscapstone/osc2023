#ifndef _P_MINI_UART_H
#define _P_MINI_UART_H

#include "peripherals/base.h"
#include "type.h"
#include "fs/vfs.h"
#define AUX_IQR (PBASE + 0x00215000)
#define AUX_ENABLES (PBASE + 0x00215004)
#define AUX_MU_IO_REG (PBASE + 0x00215040)
#define AUX_MU_IER_REG (PBASE + 0x00215044)
#define AUX_MU_IIR_REG (PBASE + 0x00215048)
#define AUX_MU_LCR_REG (PBASE + 0x0021504c)
#define AUX_MU_MCR_REG (PBASE + 0x00215050)
#define AUX_MU_LSR_REG (PBASE + 0x00215054)
#define AUX_MU_MSR_REG (PBASE + 0x00215058)
#define AUX_MU_SCRATCH (PBASE + 0x0021505c)
#define AUX_MU_CNTL_REG (PBASE + 0x00215060)
#define AUX_MU_STAT_REG (PBASE + 0x00215064)
#define AUX_MU_BAUD_REG (PBASE + 0x00215068)
#define UART_QUEUE_SIZE (40)

#define UART_ASYNC 0
#define UART_DEFAULT 1

void uart_init();
char uart_recv();
void uart_send(char c);
void uart_send_string(const char *c);
void uart_send_u32(unsigned int u32);
void uart_send_u64(unsigned long u64);
uint64_t uart_send_n(const char *c, unsigned long long n);
void uart_send_dec(unsigned long dec);
void uart_int_trans_handler();
void uart_int_recv_handler();
void enable_aux_interrupt();
void disable_aux_interrupt();
char async_uart_recv(void);
void async_uart_send(char c);
uint64_t async_uart_rx_buf(char buf[], uint64_t sz);
void async_uart_send_seting(const char *buf);
void uart_switch_func(unsigned int func);
struct Uart_MSG_Queue {
    unsigned int size;
    unsigned int begin;
    unsigned int end;
    char buf[UART_QUEUE_SIZE];
};


int uart_file_write(struct file *f, const void *buf, long len);

int uart_file_read(struct file *f, const void *buf, long len) ;

int uart_file_open(struct vnode *f, struct file **target);
int uart_file_close(struct file *f);
// static struct Uart_MSG_Queue txbuf;
// static struct Uart_MSG_Queue rxbuf;
#endif