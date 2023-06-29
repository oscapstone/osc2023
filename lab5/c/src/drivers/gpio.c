#include "oscos/drivers/gpio.h"

#include <stdint.h>

#include "oscos/drivers/board.h"
#include "oscos/timer/delay.h"

#define GPIO_REG_BASE ((void *)((char *)PERIPHERAL_BASE + 0x200000))

typedef struct {
  volatile uint32_t fsel[6];
  const volatile uint32_t _reserved0;
  volatile uint32_t set[2];
  const volatile uint32_t _reserved1;
  volatile uint32_t clr[2];
  const volatile uint32_t _reserved2;
  const volatile uint32_t lev[2];
  const volatile uint32_t _reserved3;
  volatile uint32_t eds[2];
  const volatile uint32_t _reserved4;
  volatile uint32_t ren[2];
  const volatile uint32_t _reserved5;
  volatile uint32_t fen[2];
  const volatile uint32_t _reserved6;
  volatile uint32_t hen[2];
  const volatile uint32_t _reserved7;
  volatile uint32_t len[2];
  const volatile uint32_t _reserved8;
  volatile uint32_t aren[2];
  const volatile uint32_t _reserved9;
  volatile uint32_t afen[2];
  const volatile uint32_t _reserved10;
  volatile uint32_t pud;
  volatile uint32_t pudclk[2];
} gpio_reg_t;

#define GPIO_REG ((gpio_reg_t *)GPIO_REG_BASE)

#define GPIO_FSEL_FSEL4_POSN 12
#define GPIO_FSEL_FSEL4_MASK ((uint32_t)((uint32_t)0x7 << GPIO_FSEL_FSEL4_POSN))
#define GPIO_FSEL_FSEL5_POSN 15
#define GPIO_FSEL_FSEL5_MASK ((uint32_t)((uint32_t)0x7 << GPIO_FSEL_FSEL5_POSN))
#define GPIO_FSEL_FSEL_INPUT ((uint32_t)0x0)
#define GPIO_FSEL_FSEL_ALT5 ((uint32_t)0x2)
#define GPIO_FSEL_FSEL4_INPUT                                                  \
  ((uint32_t)(GPIO_FSEL_FSEL_INPUT << GPIO_FSEL_FSEL4_POSN))
#define GPIO_FSEL_FSEL4_ALT5                                                   \
  ((uint32_t)(GPIO_FSEL_FSEL_ALT5 << GPIO_FSEL_FSEL4_POSN))
#define GPIO_FSEL_FSEL5_INPUT                                                  \
  ((uint32_t)(GPIO_FSEL_FSEL_INPUT << GPIO_FSEL_FSEL5_POSN))
#define GPIO_FSEL_FSEL5_ALT5                                                   \
  ((uint32_t)(GPIO_FSEL_FSEL_ALT5 << GPIO_FSEL_FSEL5_POSN))

#define GPIO_PUD_PUD_OFF ((uint32_t)0x0)
#define GPIO_PUD_PUD_PULL_UP ((uint32_t)0x2)

void gpio_init(void) {
  // No-op.
}

void gpio_setup_uart0_gpio14(void) {
  PERIPHERAL_WRITE_BARRIER();

  // Set GPIO pin 14 & 15 to use alternate function 5 ({T,R}XD1).
  GPIO_REG->fsel[1] =
      (GPIO_REG->fsel[1] & ~(GPIO_FSEL_FSEL4_MASK | GPIO_FSEL_FSEL5_MASK)) |
      (GPIO_FSEL_FSEL4_ALT5 | GPIO_FSEL_FSEL5_ALT5);

  // Setup the GPIO pull up/down resistors on pin 14 & 15.
  // Pin 14 (TXD1): Disabled.
  // Pin 15 (RXD1): Pull up.
  //
  // Instead of disabling the pull up/down resistors on pin 15 as specified in
  // lab 1, we instead pull it up to ensure that mini UART doesn't read in
  // garbage data when the pin is not connected.
  //
  // The delay period of 1 μs is calculated by dividing the required delay
  // period of 150 clock cycles (as specified in [bcm2835-datasheet]) by 150
  // MHz, the nominal core frequency mentioned in [bcm2835-datasheet], pp. 34.
  // We believe 150 MHz should be used instead of the actual core frequency of
  // 250 MHz because the setup/hold time of a digital circuit is usually
  // specified in terms of real time (e.g. nanoseconds) instead of in clock
  // cycles. The specified delay period of 150 clock cycles might have been
  // derived by multiplying the actual required delay period of 1 μs by the
  // nominal core frequency of 150 MHz.
  //
  // [bcm2835-datasheet]:
  //   https://datasheets.raspberrypi.com/bcm2835/bcm2835-peripherals.pdf

  GPIO_REG->pud = GPIO_PUD_PUD_OFF;
  delay_ns(1000);
  GPIO_REG->pudclk[0] = 1 << 14;
  delay_ns(1000);
  GPIO_REG->pud = 0;
  GPIO_REG->pudclk[0] = 0;

  GPIO_REG->pud = GPIO_PUD_PUD_PULL_UP;
  delay_ns(1000);
  GPIO_REG->pudclk[0] = 1 << 15;
  delay_ns(1000);
  GPIO_REG->pud = 0;
  GPIO_REG->pudclk[0] = 0;

  PERIPHERAL_READ_BARRIER();
}
