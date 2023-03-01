// Information related to the PM registers can be retrieved from [bcm2835-regs],
// #PM.
//
// [bcm2835-regs]: https://elinux.org/BCM2835_registers#PM

#ifndef OSCOS_BCM2837_PM_H
#define OSCOS_BCM2837_PM_H

#include <stdint.h>

#include "oscos/bcm2837/peripheral.h"

#define PM_BASE ((void *)((uintptr_t)PERIPHERAL_BASE + 0x100000))

typedef struct {
  volatile uint32_t GNRIC;
  volatile uint32_t AUDIO;
  const volatile uint32_t _reserved0[4];
  const volatile uint32_t STATUS;
  volatile uint32_t RSTC;
  volatile uint32_t RSTS;
  volatile uint32_t WDOG;
  volatile uint32_t PADS0;
  volatile uint32_t PADS2;
  volatile uint32_t PADS3;
  volatile uint32_t PADS4;
  volatile uint32_t PADS5;
  volatile uint32_t PADS6;
  const volatile uint32_t _reserved1;
  volatile uint32_t CAM0;
  volatile uint32_t CAM1;
  volatile uint32_t CCP2TX;
  volatile uint32_t DSI0;
  volatile uint32_t DSI1;
  volatile uint32_t HDMI;
  volatile uint32_t USB;
  volatile uint32_t PXLDO;
  volatile uint32_t PXBG;
  volatile uint32_t DFT;
  volatile uint32_t SMPS;
  volatile uint32_t XOSC;
  volatile uint32_t SPAREW;
  const volatile uint32_t SPARER;
  volatile uint32_t AVS_RSTDR;
  volatile uint32_t AVS_STAT;
  volatile uint32_t AVS_EVENT;
  volatile uint32_t AVS_INTEN;
  const volatile uint32_t _reserved2[29];
  const volatile uint32_t DUMMY;
  const volatile uint32_t _reserved3[2];
  volatile uint32_t IMAGE;
  volatile uint32_t GRAFX;
  volatile uint32_t PROC;
} PM_t;

#define PM ((PM_t *)PM_BASE)

#define PM_PASSWORD 0x5a000000

#endif
