#ifndef OSCOS_BCM2837_GPIO_H
#define OSCOS_BCM2837_GPIO_H

#include <stdint.h>

#include "oscos/bcm2837/peripheral.h"

#define GP_BASE ((void *)((uintptr_t)PERIPHERAL_BASE + 0x200000))

typedef struct {
  volatile uint32_t FSEL[6];
  const volatile uint32_t _reserved0;
  volatile uint32_t SET[2];
  const volatile uint32_t _reserved1;
  volatile uint32_t CLR[2];
  const volatile uint32_t _reserved2;
  const volatile uint32_t LEV[2];
  const volatile uint32_t _reserved3;
  volatile uint32_t EDS[2];
  const volatile uint32_t _reserved4;
  volatile uint32_t REN[2];
  const volatile uint32_t _reserved5;
  volatile uint32_t FEN[2];
  const volatile uint32_t _reserved6;
  volatile uint32_t HEN[2];
  const volatile uint32_t _reserved7;
  volatile uint32_t LEN[2];
  const volatile uint32_t _reserved8;
  volatile uint32_t AREN[2];
  const volatile uint32_t _reserved9;
  volatile uint32_t AFEN[2];
  const volatile uint32_t _reserved10;
  volatile uint32_t PUD;
  volatile uint32_t PUDCLK[2];
} GP_t;

#define GP ((GP_t *)GP_BASE)

#define GPFSEL_FSEL4_POSN 12
#define GPFSEL_FSEL4_MASK ((uint32_t)((uint32_t)0x7 << GPFSEL_FSEL4_POSN))
#define GPFSEL_FSEL5_POSN 15
#define GPFSEL_FSEL5_MASK ((uint32_t)((uint32_t)0x7 << GPFSEL_FSEL5_POSN))
#define GPFSEL_FSEL_ALT5 ((uint32_t)0x2)
#define GPFSEL_FSEL4_ALT5 ((uint32_t)(GPFSEL_FSEL_ALT5 << GPFSEL_FSEL4_POSN))
#define GPFSEL_FSEL5_ALT5 ((uint32_t)(GPFSEL_FSEL_ALT5 << GPFSEL_FSEL5_POSN))

#define GPPUD_PUD_OFF ((uint32_t)0x0)

#endif
