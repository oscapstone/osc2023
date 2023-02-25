#ifndef _MINI_UART_H
#define _MINI_UART_H

#define AUX_BASE        0x3F000000
#define AUX_ENABLES     (AUX_BASE+0x00215004)
#define AUX_MU_IO_REG   (AUX_BASE+0x00215040)
#define AUX_MU_IER_REG  (AUX_BASE+0x00215044)
#define AUX_MU_IIR_REG  (AUX_BASE+0x00215048)
#define AUX_MU_LCR_REG  (AUX_BASE+0x0021504C)
#define AUX_MU_MCR_REG  (AUX_BASE+0x00215050)
#define AUX_MU_LSR_REG  (AUX_BASE+0x00215054)
#define AUX_MU_MSR_REG  (AUX_BASE+0x00215058)
#define AUX_MU_SCRATCH  (AUX_BASE+0x0021505C)
#define AUX_MU_CNTL_REG (AUX_BASE+0x00215060)
#define AUX_MU_STAT_REG (AUX_BASE+0x00215064)
#define AUX_MU_BAUD_REG (AUX_BASE+0x00215068)

#define GPFSEL1         (AUX_BASE+0x00200004)
#define GPSET0          (AUX_BASE+0x0020001C)
#define GPCLR0          (AUX_BASE+0x00200028)
#define GPPUD           (AUX_BASE+0x00200094)
#define GPPUDCLK0       (AUX_BASE+0x00200098)

void uart_init();
char uart_recv();
void uart_send(unsigned int c);
void uart_send_string(char* str);

#endif /* _MINI_UART_H */
