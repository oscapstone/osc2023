#ifndef _P_MINI_UART
#define _P_MINI_UART

#define PBASE 0x3F000000
// use phy addr 0x3f instead of bus addr 0x7e
#define AUX_ENABLES     ((volatile unsigned int*)(PBASE+0x00215004))
#define AUX_MU_CNTL_REG ((volatile unsigned int*)(PBASE+0x00215060))
#define AUX_MU_IER_REG  ((volatile unsigned int*)(PBASE+0x00215044))
#define AUX_MU_LCR_REG  ((volatile unsigned int*)(PBASE+0x0021504C))
#define AUX_MU_MCR_REG  ((volatile unsigned int*)(PBASE+0x00215050))
#define AUX_MU_BAUD_REG ((volatile unsigned int*)(PBASE+0x00215068))
#define AUX_MU_IIR_REG  ((volatile unsigned int*)(PBASE+0x00215048))
#define AUX_MU_LSR_REG  ((volatile unsigned int*)(PBASE+0x00215054))
#define AUX_MU_IO_REG   ((volatile unsigned int*)(PBASE+0x00215040))
#define AUX_MU_MSR_REG  ((volatile unsigned int*)(PBASE+0x00215058))
#define AUX_MU_SCRATCH_REG  ((volatile unsigned int*)(PBASE+0x0021505C))
#define AUX_MU_STAT_REG   ((volatile unsigned int*)(PBASE+0x00215064))
// see https://oscapstone.github.io/osc2022/labs/hardware/uart.html#uart
#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

#define GET_BOARD_REVISION  0x00010002
#define REQUEST_CODE        0x00000000
#define REQUEST_SUCCEED     0x80000000
#define REQUEST_FAILED      0x80000001
#define TAG_REQUEST_CODE    0x00000000
#define END_TAG             0x00000000

#endif
