#include "oscos/drivers/pm.h"

#include <stdint.h>

#include "oscos/drivers/board.h"

// Information related to the PM registers can be retrieved from [bcm2835-regs],
// #PM.
//
// [bcm2835-regs]: https://elinux.org/BCM2835_registers#PM

#define PM_REG_BASE ((void *)((char *)PERIPHERAL_BASE + 0x100000))

typedef struct {
  volatile uint32_t gnric;
  volatile uint32_t audio;
  const volatile uint32_t _reserved0[4];
  const volatile uint32_t status;
  volatile uint32_t rstc;
  volatile uint32_t rsts;
  volatile uint32_t wdot;
  volatile uint32_t pads0;
  volatile uint32_t pads2;
  volatile uint32_t pads3;
  volatile uint32_t pads4;
  volatile uint32_t pads5;
  volatile uint32_t pads6;
  const volatile uint32_t _reserved1;
  volatile uint32_t cam0;
  volatile uint32_t cam1;
  volatile uint32_t cpp2tx;
  volatile uint32_t dsi0;
  volatile uint32_t dsi1;
  volatile uint32_t hdmi;
  volatile uint32_t usb;
  volatile uint32_t pxldo;
  volatile uint32_t pxbg;
  volatile uint32_t dft;
  volatile uint32_t smps;
  volatile uint32_t xosc;
  volatile uint32_t sparew;
  const volatile uint32_t sparer;
  volatile uint32_t avs_rstdr;
  volatile uint32_t avs_stat;
  volatile uint32_t avs_event;
  volatile uint32_t avs_inten;
  const volatile uint32_t _reserved2[29];
  const volatile uint32_t dummy;
  const volatile uint32_t _reserved3[2];
  volatile uint32_t image;
  volatile uint32_t grafx;
  volatile uint32_t proc;
} pm_reg_t;

#define PM_REG ((pm_reg_t *)PM_REG_BASE)

#define PM_PASSWORD 0x5a000000

void pm_init(void) {
  // No-op.
}

void pm_reset(const uint32_t tick) {
  PERIPHERAL_WRITE_BARRIER();

  PM_REG->rstc = PM_PASSWORD | 0x20;
  PM_REG->wdot = PM_PASSWORD | tick;

  PERIPHERAL_READ_BARRIER();
}

void pm_cancel_reset(void) {
  PERIPHERAL_WRITE_BARRIER();

  PM_REG->rstc = PM_PASSWORD | 0;
  PM_REG->wdot = PM_PASSWORD | 0;

  PERIPHERAL_READ_BARRIER();
}

noreturn void pm_reboot(void) {
  pm_reset(1);
  for (;;)
    ;
}
