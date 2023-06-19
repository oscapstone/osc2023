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
static void (*volatile _console_read_notify_callback)(
    void *) = NULL,
         (*volatile _console_write_notify_callback)(void *) = NULL;
static void *volatile _console_read_notify_callback_arg,
    *volatile _console_write_notify_callback_arg;

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

  // Send notification.

  if (_console_read_buf_len != 0) {
    void (*const callback)(void *) = _console_read_notify_callback;
    _console_read_notify_callback = NULL;

    if (callback) {
      callback(_console_read_notify_callback_arg);
    }
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

  // Send notification.

  if (_console_write_buf_len != WRITE_BUF_SZ) {
    void (*const callback)(void *) = _console_write_notify_callback;
    _console_write_notify_callback = NULL;

    if (callback) {
      callback(_console_write_notify_callback_arg);
    }
  }
}

// Raw operations.

static int _console_recv_byte_nonblock(void) {
  if (_console_read_buf_len == 0)
    return -1;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  const unsigned char result = _console_read_buf[_console_read_buf_start];
  _console_read_buf_start = (_console_read_buf_start + 1) % READ_BUF_SZ;
  _console_read_buf_len--;

  CRITICAL_SECTION_LEAVE(daif_val);

  // Re-enable receive interrupt.
  mini_uart_enable_rx_interrupt();

  return result;
}

static bool _console_send_byte_nonblock(const unsigned char b) {
  if (_console_write_buf_len == WRITE_BUF_SZ)
    return false;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  _console_write_buf[(_console_write_buf_start + _console_write_buf_len++) %
                     WRITE_BUF_SZ] = b;

  CRITICAL_SECTION_LEAVE(daif_val);

  // Re-enable transmit interrupt.
  mini_uart_enable_tx_interrupt();

  return true;
}

static bool _console_send_two_bytes_nonblock(const unsigned char b1,
                                             const unsigned char b2) {
  if (_console_write_buf_len > WRITE_BUF_SZ - 2)
    return false;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  _console_write_buf[(_console_write_buf_start + _console_write_buf_len++) %
                     WRITE_BUF_SZ] = b1;
  _console_write_buf[(_console_write_buf_start + _console_write_buf_len++) %
                     WRITE_BUF_SZ] = b2;

  CRITICAL_SECTION_LEAVE(daif_val);

  // Re-enable transmit interrupt.
  mini_uart_enable_tx_interrupt();

  return true;
}

// Mode switching is implemented by switching the vtables for the two primitive
// operations, getc and putc.

typedef struct {
  int (*getc_nonblock)(void);
  bool (*putc_nonblock)(unsigned char);
} console_primop_vtable_t;

static int _console_getc_nonblock_text_mode(void) {
  const int c = _console_recv_byte_nonblock();
  if (c < 0)
    return c;
  return c == '\r' ? '\n' : c;
}

static int _console_getc_nonblock_binary_mode(void) {
  return _console_recv_byte_nonblock();
}

static bool _console_putc_nonblock_text_mode(const unsigned char c) {
  if (c == '\n') {
    return _console_send_two_bytes_nonblock('\r', '\n');
  } else {
    return _console_send_byte_nonblock(c);
  }
}

static bool _console_putc_nonblock_binary_mode(const unsigned char c) {
  return _console_send_byte_nonblock(c);
}

static const console_primop_vtable_t
    _primop_vtable_text_mode = {.getc_nonblock =
                                    _console_getc_nonblock_text_mode,
                                .putc_nonblock =
                                    _console_putc_nonblock_text_mode},
    _primop_vtable_binary_mode = {.getc_nonblock =
                                      _console_getc_nonblock_binary_mode,
                                  .putc_nonblock =
                                      _console_putc_nonblock_binary_mode},
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

unsigned char console_getc(void) {
  int read_result = -1;
  uint64_t daif_val;

  while (read_result < 0) {
    CRITICAL_SECTION_ENTER(daif_val);

    if ((read_result = console_getc_nonblock()) < 0) {
      __asm__ __volatile__("wfi");
      _console_recv_to_buf();
    }

    CRITICAL_SECTION_LEAVE(daif_val);
  }

  return read_result;
}

int console_getc_nonblock(void) { return _primop_vtable->getc_nonblock(); }

size_t console_read_nonblock(void *const buf, const size_t count) {
  unsigned char *const buf_c = (unsigned char *)buf;

  size_t n_chars_read;
  int read_result;

  for (n_chars_read = 0;
       n_chars_read < count && (read_result = console_getc_nonblock()) >= 0;
       n_chars_read++) {
    buf_c[n_chars_read] = read_result;
  }

  return n_chars_read;
}

unsigned char console_putc(const unsigned char c) {
  bool putc_successful = false;
  uint64_t daif_val;

  while (!putc_successful) {
    CRITICAL_SECTION_ENTER(daif_val);

    if (!(putc_successful = console_putc_nonblock(c) >= 0)) {
      __asm__ __volatile__("wfi");
      _console_send_from_buf();
    }

    CRITICAL_SECTION_LEAVE(daif_val);
  }

  return c;
}

int console_putc_nonblock(const unsigned char c) {
  const bool putc_successful = _primop_vtable->putc_nonblock(c);
  return putc_successful ? c : -1;
}

size_t console_write(const void *const buf, const size_t count) {
  const unsigned char *const buf_c = buf;
  for (size_t i = 0; i < count; i++) {
    console_putc(buf_c[i]);
  }
  return count;
}

size_t console_write_nonblock(const void *const buf, const size_t count) {
  const unsigned char *const buf_c = buf;

  size_t n_chars_written;
  bool putc_successful;

  for (n_chars_written = 0;
       n_chars_written < count &&
       (putc_successful = console_putc_nonblock(buf_c[n_chars_written]) >= 0);
       n_chars_written++)
    ;

  return n_chars_written;
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

bool console_notify_read_ready(void (*const callback)(void *),
                               void *const arg) {
  if (_console_read_notify_callback)
    return false;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  _console_read_notify_callback = callback;
  _console_read_notify_callback_arg = arg;

  CRITICAL_SECTION_LEAVE(daif_val);
  return true;
}

bool console_notify_write_ready(void (*const callback)(void *),
                                void *const arg) {
  if (_console_write_notify_callback)
    return false;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  _console_write_notify_callback = callback;
  _console_write_notify_callback_arg = arg;

  CRITICAL_SECTION_LEAVE(daif_val);
  return true;
}

void mini_uart_interrupt_handler(void) {
  _console_recv_to_buf();
  _console_send_from_buf();
}
