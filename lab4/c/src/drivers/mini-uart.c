#include "oscos/drivers/mini-uart.h"

#include <stdint.h>

#include "oscos/drivers/aux.h"
#include "oscos/drivers/board.h"

#define MINI_UART_REG_BASE ((void *)((char *)PERIPHERAL_BASE + 0x215040))

typedef struct {
  volatile uint32_t io;
  volatile uint32_t ier;
  volatile uint32_t iir;
  volatile uint32_t lcr;
  volatile uint32_t mcr;
  const volatile uint32_t lsr;
  const volatile uint32_t msr;
  volatile uint32_t scratch;
  volatile uint32_t cntl;
  const volatile uint32_t stat;
  volatile uint32_t baud;
} mini_uart_reg_t;

#define MINI_UART_REG ((mini_uart_reg_t *)MINI_UART_REG_BASE)

// N. B. The interrupt bits specified in the datasheet are incorrect. The values
// listed here are correct. See [bcm2837-datasheet-errata].
// [bcm2835-datasheet-errata]: https://elinux.org/BCM2835_datasheet_errata

#define MINI_UART_IER_ENABLE_RECEIVE_INTERRUPT ((uint32_t)(1 << 0))
#define MINI_UART_IER_ENABLE_TRANSMIT_INTERRUPT ((uint32_t)(1 << 1))

#define MINI_UART_LSR_DATA_READY ((uint32_t)(1 << 0))
#define MINI_UART_LSR_TRANSMITTER_IDLE ((uint32_t)(1 << 6))
#define MINI_UART_LSR_TRANSMITTER_EMPTY ((uint32_t)(1 << 5))

void mini_uart_init(void) {
  // The initialization procedure is taken from
  // https://oscapstone.github.io/labs/hardware/uart.html.

  // Enable mini UART.
  aux_enable_mini_uart();

  PERIPHERAL_WRITE_BARRIER();

  // Disable TX and RX.
  MINI_UART_REG->cntl = 0;
  // Disable interrupt.
  MINI_UART_REG->ier = 0;
  // Set the data size to 8 bits.
  // N. B. The datasheet [bcm2835-datasheet] incorrectly indicates that only bit
  // 0 needs to be set. In fact, bits [1:0] need to be set to 3. See
  // [bcm2835-datasheet-errata], #p14.
  MINI_UART_REG->lcr = 3;
  // Disable auto flow control.
  MINI_UART_REG->mcr = 0;
  // Set baud rate to 115200.
  MINI_UART_REG->baud = 270;
  // Clear the transmit and receive FIFOs.
  MINI_UART_REG->iir = 6;
  // Enable TX and RX.
  MINI_UART_REG->cntl = 3;

  PERIPHERAL_READ_BARRIER();
}

int mini_uart_recv_byte_nonblock(void) {
  int result;

  if (!(MINI_UART_REG->lsr & MINI_UART_LSR_DATA_READY)) {
    result = -1;
    goto end;
  }

  result = (unsigned char)MINI_UART_REG->io;

end:
  PERIPHERAL_READ_BARRIER();
  return result;
}

int mini_uart_send_byte_nonblock(const unsigned char b) {
  PERIPHERAL_WRITE_BARRIER();

  int result;

  if (!(MINI_UART_REG->lsr & MINI_UART_LSR_TRANSMITTER_EMPTY)) {
    result = -1;
    goto end;
  }

  MINI_UART_REG->io = b;
  result = 0;

end:
  PERIPHERAL_READ_BARRIER();
  return result;
}

void mini_uart_enable_rx_interrupt(void) {
  PERIPHERAL_WRITE_BARRIER();

  MINI_UART_REG->ier |= MINI_UART_IER_ENABLE_RECEIVE_INTERRUPT;

  PERIPHERAL_READ_BARRIER();
}

void mini_uart_disable_rx_interrupt(void) {
  PERIPHERAL_WRITE_BARRIER();

  MINI_UART_REG->ier &= ~MINI_UART_IER_ENABLE_RECEIVE_INTERRUPT;

  PERIPHERAL_READ_BARRIER();
}

void mini_uart_enable_tx_interrupt(void) {
  PERIPHERAL_WRITE_BARRIER();

  MINI_UART_REG->ier |= MINI_UART_IER_ENABLE_TRANSMIT_INTERRUPT;

  PERIPHERAL_READ_BARRIER();
}

void mini_uart_disable_tx_interrupt(void) {
  PERIPHERAL_WRITE_BARRIER();

  MINI_UART_REG->ier &= ~MINI_UART_IER_ENABLE_TRANSMIT_INTERRUPT;

  PERIPHERAL_READ_BARRIER();
}
