#ifndef __PERIPHERALS_UART_H__
#define __PERIPHERALS_UART_H__

#include "peripherals/base.h"

// clang-format off
#define AUX_IRQ              ((volatile unsigned int*)(MMIO_BASE + 0x00215000))
#define AUX_ENABLES          ((volatile unsigned int*)(MMIO_BASE + 0x00215004))
#define AUX_MU_IO_REG        ((volatile unsigned int*)(MMIO_BASE + 0x00215040))
#define AUX_MU_IER_REG       ((volatile unsigned int*)(MMIO_BASE + 0x00215044))
#define AUX_MU_IIR_REG       ((volatile unsigned int*)(MMIO_BASE + 0x00215048))
#define AUX_MU_LCR_REG       ((volatile unsigned int*)(MMIO_BASE + 0x0021504C))
#define AUX_MU_MCR_REG       ((volatile unsigned int*)(MMIO_BASE + 0x00215050))
#define AUX_MU_LSR_REG       ((volatile unsigned int*)(MMIO_BASE + 0x00215054))
#define AUX_MU_MSR_REG       ((volatile unsigned int*)(MMIO_BASE + 0x00215058))
#define AUX_MU_SCRATCH       ((volatile unsigned int*)(MMIO_BASE + 0x0021505C))
#define AUX_MU_CNTL_REG      ((volatile unsigned int*)(MMIO_BASE + 0x00215060))
#define AUX_MU_STAT_REG      ((volatile unsigned int*)(MMIO_BASE + 0x00215064))
#define AUX_MU_BAUD_REG      ((volatile unsigned int*)(MMIO_BASE + 0x00215068))
#ifdef RPI4
#define AUX_SPI1_CNTL0_REG   ((volatile unsigned int*)(MMIO_BASE + 0x00215080))
#define AUX_SPI1_CNTL1_REG   ((volatile unsigned int*)(MMIO_BASE + 0x00215084))
#define AUX_SPI1_STAT_REG    ((volatile unsigned int*)(MMIO_BASE + 0x00215088))
#define AUX_SPI1_PEEK_REG    ((volatile unsigned int*)(MMIO_BASE + 0x0021508C))
#define AUX_SPI1_IO_REGa     ((volatile unsigned int*)(MMIO_BASE + 0x002150A0))
#define AUX_SPI1_IO_REGb     ((volatile unsigned int*)(MMIO_BASE + 0x002150A4))
#define AUX_SPI1_IO_REGc     ((volatile unsigned int*)(MMIO_BASE + 0x002150A8))
#define AUX_SPI1_IO_REGd     ((volatile unsigned int*)(MMIO_BASE + 0x002150AC))
#define AUX_SPI1_TXHOLD_REGa ((volatile unsigned int*)(MMIO_BASE + 0x002150B0))
#define AUX_SPI1_TXHOLD_REGb ((volatile unsigned int*)(MMIO_BASE + 0x002150B4))
#define AUX_SPI1_TXHOLD_REGc ((volatile unsigned int*)(MMIO_BASE + 0x002150B8))
#define AUX_SPI1_TXHOLD_REGd ((volatile unsigned int*)(MMIO_BASE + 0x002150BC))
#define AUX_SPI2_CNTL0_REG   ((volatile unsigned int*)(MMIO_BASE + 0x002150C0))
#define AUX_SPI2_CNTL1_REG   ((volatile unsigned int*)(MMIO_BASE + 0x002150C4))
#define AUX_SPI2_STAT_REG    ((volatile unsigned int*)(MMIO_BASE + 0x002150C8))
#define AUX_SPI2_PEEK_REG    ((volatile unsigned int*)(MMIO_BASE + 0x002150CC))
#define AUX_SPI2_IO_REGa     ((volatile unsigned int*)(MMIO_BASE + 0x002150E0))
#define AUX_SPI2_IO_REGb     ((volatile unsigned int*)(MMIO_BASE + 0x002150E4))
#define AUX_SPI2_IO_REGc     ((volatile unsigned int*)(MMIO_BASE + 0x002150E8))
#define AUX_SPI2_IO_REGd     ((volatile unsigned int*)(MMIO_BASE + 0x002150EC))
#define AUX_SPI2_TXHOLD_REGa ((volatile unsigned int*)(MMIO_BASE + 0x002150F0))
#define AUX_SPI2_TXHOLD_REGb ((volatile unsigned int*)(MMIO_BASE + 0x002150F4))
#define AUX_SPI2_TXHOLD_REGc ((volatile unsigned int*)(MMIO_BASE + 0x002150F8))
#define AUX_SPI2_TXHOLD_REGd ((volatile unsigned int*)(MMIO_BASE + 0x002150FC))
#elif RPI3
#define AUX_SPI0_CNTL0_REG   ((volatile unsigned int*)(MMIO_BASE + 0x00215080))
#define AUX_SPI0_CNTL1_REG   ((volatile unsigned int*)(MMIO_BASE + 0x00215084))
#define AUX_SPI0_STAT_REG    ((volatile unsigned int*)(MMIO_BASE + 0x00215088))
#define AUX_SPI0_IO_REG      ((volatile unsigned int*)(MMIO_BASE + 0x00215090))
#define AUX_SPI0_PEEK_REG    ((volatile unsigned int*)(MMIO_BASE + 0x00215094))
#define AUX_SPI1_CNTL0_REG   ((volatile unsigned int*)(MMIO_BASE + 0x002150C0))
#define AUX_SPI1_CNTL1_REG   ((volatile unsigned int*)(MMIO_BASE + 0x002150C4))
#define AUX_SPI1_STAT_REG    ((volatile unsigned int*)(MMIO_BASE + 0x002150C8))
#define AUX_SPI1_IO_REG      ((volatile unsigned int*)(MMIO_BASE + 0x002150D0))
#define AUX_SPI1_PEEK_REG    ((volatile unsigned int*)(MMIO_BASE + 0x002150D4))
#endif
// clang-format on

#endif
