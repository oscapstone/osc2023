#ifndef BCM2835_UART
#define BCM2835_UART

#include "bcm2835/addr.h"

#define AUX_ENABLES     PRPHRL(0x215004U)
#define AUX_MU_IO_REG   PRPHRL(0x215040U)
#define AUX_MU_IER_REG  PRPHRL(0x215044U)
#define AUX_MU_IIR_REG  PRPHRL(0x215048U)
#define AUX_MU_LCR_REG  PRPHRL(0x21504CU)
#define AUX_MU_MCR_REG  PRPHRL(0x215050U)
#define AUX_MU_LSR_REG  PRPHRL(0x215054U)
#define AUX_MU_CNTL_REG PRPHRL(0x215060U)
#define AUX_MU_BAUD_REG PRPHRL(0x215068U)

void uart_init();
char uart_recv();
void uart_send(char c);
void uart_puts(char * s);
void uart_pu32h(unsigned int n);

#endif