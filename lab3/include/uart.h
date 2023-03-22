#include "gpio.h"
#include <stdint.h>

#ifndef UART_H
#define UART_H

#define PHY_AUX_IRQ 0x7e215000
#define PHY_AUX_ENABLE 0x7e215004
#define PHY_AUX_MU_IO 0x7e215040
#define PHY_AUX_MU_IER 0x7e215044
#define PHY_AUX_MU_IIR 0x7e215048
#define PHY_AUX_MU_LCR 0x7e21504c
#define PHY_AUX_MU_MCR 0x7e215050
#define PHY_AUX_MU_LSR 0x7e215054
#define PHY_AUX_MU_MSR 0x7e215058
#define PHY_AUX_MU_SCRATCH 0x7e21505c
#define PHY_AUX_MU_CNTL 0x7e215060
#define PHY_AUX_MU_STAT 0x7e215064
#define PHY_AUX_MU_BAUD 0x7e215068

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

void uart_setup(void);
void uart_send(unsigned int);
char uart_get(void);
char uart_getc(void);         // get() + '\r' -> '\n'
void uart_putc(char);         // send() + '\n' -> '\r'
void uart_puts(char *s);      // Print string to '\0'
void uart_puti(unsigned int); // For dec output
void uart_puth(unsigned int);
void uart_puthl(uint64_t); // For 64-bit hex
void uart_gets(char *s);
void uart_putsn(char *s, int n); // only print n chars ignore '\0'
void read_kernel();
int uart_transmit_handler();
int uart_receive_handler();
int uart_a_puts(const char*, int);
int uart_a_gets(char*, int);


#endif // UART_H
