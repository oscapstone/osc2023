#include "gpio.h"
#include <stdint.h>

#ifndef UART_H
#define UART_H

#define PHY_AUX_IRQ 0xffff00007e215000
#define PHY_AUX_ENABLE 0xffff00007e215004
#define PHY_AUX_MU_IO 0xffff00007e215040
#define PHY_AUX_MU_IER 0xffff00007e215044
#define PHY_AUX_MU_IIR 0xffff00007e215048
#define PHY_AUX_MU_LCR 0xffff00007e21504c
#define PHY_AUX_MU_MCR 0xffff00007e215050
#define PHY_AUX_MU_LSR 0xffff00007e215054
#define PHY_AUX_MU_MSR 0xffff00007e215058
#define PHY_AUX_MU_SCRATCH 0xffff00007e21505c
#define PHY_AUX_MU_CNTL 0xffff00007e215060
#define PHY_AUX_MU_STAT 0xffff00007e215064
#define PHY_AUX_MU_BAUD 0xffff00007e215068

#define AUX_IRQ                                                                \
  (volatile unsigned int *)((PHY_AUX_IRQ) - (BUS_BASE) + (MMIO_BASE))
#define AUX_ENABLE                                                             \
  (volatile unsigned int *)((PHY_AUX_ENABLE) - (BUS_BASE) + (MMIO_BASE))
#define AUX_MU_IO                                                              \
  (volatile unsigned int *)((PHY_AUX_MU_IO) - (BUS_BASE) + (MMIO_BASE))
#define AUX_MU_IER                                                             \
  (volatile unsigned int *)((PHY_AUX_MU_IER) - (BUS_BASE) + (MMIO_BASE))
#define AUX_MU_IIR                                                             \
  (volatile unsigned int *)((PHY_AUX_MU_IIR) - (BUS_BASE) + (MMIO_BASE))
#define AUX_MU_LCR                                                             \
  (volatile unsigned int *)((PHY_AUX_MU_LCR) - (BUS_BASE) + (MMIO_BASE))
#define AUX_MU_MCR                                                             \
  (volatile unsigned int *)((PHY_AUX_MU_MCR) - (BUS_BASE) + (MMIO_BASE))
#define AUX_MU_LSR                                                             \
  (volatile unsigned int *)((PHY_AUX_MU_LSR) - (BUS_BASE) + (MMIO_BASE))
#define AUX_MU_MS                                                              \
  (volatile unsigned int *)((PHY_AUX_MU_MS) - (BUS_BASE) + (MMIO_BASE))
#define AUX_MU_SCRATCH                                                         \
  (volatile unsigned int *)((PHY_AUX_MU_SCRATCH) - (BUS_BASE) + (MMIO_BASE))
#define AUX_MU_CNTL                                                            \
  (volatile unsigned int *)((PHY_AUX_MU_CNTL) - (BUS_BASE) + (MMIO_BASE))
#define AUX_MU_STA                                                             \
  (volatile unsigned int *)((PHY_AUX_MU_STA) - (BUS_BASE) + (MMIO_BASE))
#define AUX_MU_BAUD                                                            \
  (volatile unsigned int *)((PHY_AUX_MU_BAUD) - (BUS_BASE) + (MMIO_BASE))

// The default BUF_LEN of the uart.
#ifndef BUF_LEN
#define BUF_LEN 256
#endif

// Uart R/W functions
void uart_setup(void);
void uart_send(unsigned int);
char uart_get(void);
char uart_getc(void);         	// get() + '\r' -> '\n'
int uart_gets(char *);		// getline
void uart_putc(char);         	// send() + '\n' -> '\r'
void uart_puts(const char *s);      	// Print string to '\0'
void uart_puti(unsigned int); 	// For dec output
void uart_puth(unsigned int);
void uart_puthl(uint64_t); 	// For 64-bit hex
void uart_putsn(char *s, int n); // only print n chars ignore '\0'
void read_kernel();
int uart_transmit_handler();
int uart_receive_handler();
int uart_a_puts(const char *, int);
int uart_a_gets(char *, int);

// Enable uart recieve handler
int enable_uart_receive_int(void);
int disable_uart_receive_int(void);
int enable_uart_transmit_int(void);
int disable_uart_transmit_int(void);

// Helper function
int reset_rx(void);

/// For async read
typedef struct {
  char *str;
  int len;
} a_string;

#endif // UART_H
