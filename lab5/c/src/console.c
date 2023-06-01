#include "oscos/console.h"

#include <stdbool.h>

#include "oscos/drivers/gpio.h"
#include "oscos/drivers/l2ic.h"
#include "oscos/drivers/mini-uart.h"
#include "oscos/mem/startup-alloc.h"
#include "oscos/utils/critical-section.h"
#include "oscos/utils/fmt.h"
#include "oscos/xcpt.h"

#define READ_BUF_SZ 1024
#define WRITE_BUF_SZ 1024

static volatile unsigned char *_console_read_buf, *_console_write_buf;
static volatile size_t _console_read_buf_start = 0, _console_read_buf_len = 0,
                       _console_write_buf_start = 0, _console_write_buf_len = 0;

// Buffer operations.

static void _console_recv_to_buf(void) {
  // Read until the read buffer is full or there are nothing to read.

  int read_result;
  while (_console_read_buf_len != READ_BUF_SZ &&
         (read_result = mini_uart_recv_byte_nonblock()) >= 0) {
    _console_read_buf[(_console_read_buf_start + _console_read_buf_len++) %
                      READ_BUF_SZ] = read_result;
  }

  // Disable the receive interrupt if the read buffer is full. Otherwise, the
  // interrupt will fire again and again after exception return, blocking the
  // main code from executing.
  if (_console_read_buf_len == READ_BUF_SZ) {
    mini_uart_disable_rx_interrupt();
  }
}

static void _console_send_from_buf(void) {
  while (_console_write_buf_len != 0 &&
         mini_uart_send_byte_nonblock(
             _console_write_buf[_console_write_buf_start]) >= 0) {
    _console_write_buf_start = (_console_write_buf_start + 1) % WRITE_BUF_SZ;
    _console_write_buf_len--;
  }

  // Disable the transmit interrupt if the write buffer is full. Otherwise, the
  // interrupt will fire again and again after exception return, blocking the
  // main code from executing.
  if (_console_write_buf_len == 0) {
    mini_uart_disable_tx_interrupt();
  }
}

// Raw operations.

static unsigned char _console_recv_byte(void) {
  bool suspend_cond_val = true;
  uint64_t daif_val;

  // Wait for data in the read buffer.
  while (suspend_cond_val) {
    CRITICAL_SECTION_ENTER(daif_val);

    if ((suspend_cond_val = _console_read_buf_len == 0)) {
      __asm__ __volatile__("wfi");
      _console_recv_to_buf();
    }

    CRITICAL_SECTION_LEAVE(daif_val);
  }

  CRITICAL_SECTION_ENTER(daif_val);

  const unsigned char result = _console_read_buf[_console_read_buf_start];
  _console_read_buf_start = (_console_read_buf_start + 1) % READ_BUF_SZ;
  _console_read_buf_len--;

  CRITICAL_SECTION_LEAVE(daif_val);

  // Re-enable receive interrupt.
  mini_uart_enable_rx_interrupt();

  return result;
}

static void _console_send_byte(const unsigned char b) {
  bool suspend_cond_val = true;
  uint64_t daif_val;

  // Wait for the write buffer to clear.
  while (suspend_cond_val) {
    CRITICAL_SECTION_ENTER(daif_val);

    if ((suspend_cond_val = _console_write_buf_len == WRITE_BUF_SZ)) {
      __asm__ __volatile__("wfi");
      _console_send_from_buf();
    }

    CRITICAL_SECTION_LEAVE(daif_val);
  }

  CRITICAL_SECTION_ENTER(daif_val);

  _console_write_buf[(_console_write_buf_start + _console_write_buf_len++) %
                     WRITE_BUF_SZ] = b;

  CRITICAL_SECTION_LEAVE(daif_val);

  // Re-enable transmit interrupt.
  mini_uart_enable_tx_interrupt();
}

// Mode switching is implemented by switching the vtables for the two primitive
// operations, getc and putc.

typedef struct {
  unsigned char (*getc)(void);
  void (*putc)(unsigned char);
} console_primop_vtable_t;

static unsigned char _console_getc_text_mode(void) {
  const unsigned char c = _console_recv_byte();
  return c == '\r' ? '\n' : c;
}

static unsigned char _console_getc_binary_mode(void) {
  return _console_recv_byte();
}

static void _console_putc_text_mode(const unsigned char c) {
  if (c == '\n') {
    _console_send_byte('\r');
    _console_send_byte('\n');
  } else {
    _console_send_byte(c);
  }
}

static void _console_putc_binary_mode(const unsigned char c) {
  _console_send_byte(c);
}

static const console_primop_vtable_t
    _primop_vtable_text_mode = {.getc = _console_getc_text_mode,
                                .putc = _console_putc_text_mode},
    _primop_vtable_binary_mode = {.getc = _console_getc_binary_mode,
                                  .putc = _console_putc_binary_mode},
    *_primop_vtable;

// Public functions.

void console_init(void) {
  gpio_setup_uart0_gpio14();
  mini_uart_init();

  console_set_mode(CM_TEXT);

  // Enable receive interrupt.
  mini_uart_enable_rx_interrupt();
  // Enable AUX interrupt on the L2 interrupt controller.
  l2ic_enable_irq_0(INT_L2_IRQ_0_SRC_AUX);

  // Allocate the buffers.
  _console_read_buf = startup_alloc(READ_BUF_SZ * sizeof(unsigned char));
  _console_write_buf = startup_alloc(WRITE_BUF_SZ * sizeof(unsigned char));
}

void console_set_mode(const console_mode_t mode) {
  // ? Should we check the value of `mode`?
  // * FIXME: If the check is implemented, update the documentation in
  // * `include/oscos/console.h`.

  switch (mode) {
  case CM_TEXT:
    _primop_vtable = &_primop_vtable_text_mode;
    break;

  case CM_BINARY:
    _primop_vtable = &_primop_vtable_binary_mode;
    break;

  default:
    __builtin_unreachable();
  }
}

unsigned char console_getc(void) { return _primop_vtable->getc(); }

unsigned char console_putc(const unsigned char c) {
  _primop_vtable->putc(c);
  return c;
}

size_t console_write(const void *const buf, const size_t count) {
  const unsigned char *const buf_c = buf;
  for (size_t i = 0; i < count; i++) {
    console_putc(buf_c[i]);
  }
  return count;
}

void console_fputs(const char *const s) {
  for (const char *c = s; *c; c++) {
    console_putc(*c);
  }
}

void console_puts(const char *const s) {
  console_fputs(s);
  console_putc('\n');
}

int console_printf(const char *const restrict format, ...) {
  va_list ap;
  va_start(ap, format);

  const int result = console_vprintf(format, ap);

  va_end(ap);
  return result;
}

static void _console_printf_putc(const unsigned char c, void *const _arg) {
  (void)_arg;

  console_putc(c);
}

static void _console_printf_finalize(void *const _arg) { (void)_arg; }

static const printf_vtable_t _console_printf_vtable = {
    .putc = _console_printf_putc, .finalize = _console_printf_finalize};

int console_vprintf(const char *const restrict format, va_list ap) {
  return vprintf_generic(&_console_printf_vtable, NULL, format, ap);
}

void mini_uart_interrupt_handler(void) {
  _console_recv_to_buf();
  _console_send_from_buf();
}
