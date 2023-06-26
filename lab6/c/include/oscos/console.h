/// \file include/oscos/console.h
/// \brief Serial console.
///
/// The serial console reads and writes data to the mini UART.
///
/// Before using the serial console, it must be initialized by calling
/// console_init(void) exactly once. Initializing the serial console twice or
/// operating on the serial console before initialization are not checked and
/// may have unintended consequences.
///
/// The serial console implements two modes: text mode and binary mode. In text
/// mode, automatic newline translation is performed. A "\r" character received
/// by the mini UART will be read as "\n", and writing "\n" to the serial
/// console will send "\r\n" down the mini UART. In binary mode, automatic
/// newline translation is not performed. Any byte received by the mini UART
/// will be read as-is, and every character written to the serial console will
/// also be sent as-is. Upon initialization, the mode is set to text mode.

#ifndef OSCOS_CONSOLE_H
#define OSCOS_CONSOLE_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

/// \brief The mode of the serial console.
typedef enum {
  CM_TEXT,  ///< Text mode. Performs newline translation.
  CM_BINARY ///< Binary mode. Every byte are sent/received as-is.
} console_mode_t;

/// \brief Initializes the serial console.
///
/// See the file-level documentation for requirements on initialization.
void console_init(void);

/// \brief Sets the mode of the serial console.
///
/// \param mode The mode. Must be `CM_TEXT` or `CM_BINARY`.
void console_set_mode(console_mode_t mode);

/// \brief Reads a character from the serial console.
///
/// When calling this function, the serial console must be initialized.
unsigned char console_getc(void);

/// \brief Reads a character from the serial console without blocking.
///
/// When calling this function, the serial console must be initialized.
///
/// \return The character read, or a negative number if the read would block.
int console_getc_nonblock(void);

/// \brief Reads as many characters from the serial console as possible without
///        blocking.
///
/// When calling this function, the serial console must be initialized.
///
/// \return The number of characters read.
size_t console_read_nonblock(void *buf, size_t count);

/// \brief Writes a character to the serial console.
///
/// When calling this function, the serial console must be initialized.
///
/// \return \p c
unsigned char console_putc(unsigned char c);

/// \brief Writes a character to the serial console without blocking.
///
/// When calling this function, the serial console must be initialized.
///
/// \return \p c if the operation is successful, or a negative number if the
///         write operation would block.
int console_putc_nonblock(unsigned char c);

/// \brief Writes characters to the serial console.
///
/// When calling this function, the serial console must be initialized.
///
/// \return \p count
size_t console_write(const void *buf, size_t count);

/// \brief Writes as many characters to the serial console as possible without
///        blocking.
///
/// When calling this function, the serial console must be initialized.
///
/// \return The number of characters written.
size_t console_write_nonblock(const void *buf, size_t count);

/// \brief Writes a string to the serial console without trailing newline.
///
/// When calling this function, the serial console must be initialized.
void console_fputs(const char *s);

/// \brief Writes a string to the serial console with trailing newline.
///
/// When calling this function, the serial console must be initialized.
void console_puts(const char *s);

/// \brief Performs string formatting and writes the result to the serial
///        console.
///
/// When calling this function, the serial console must be initialized.
///
/// \return The number of characters written.
int console_vprintf(const char *restrict format, va_list ap)
    __attribute__((format(printf, 1, 0)));

/// \brief Performs string formatting and writes the result to the serial
///        console.
///
/// When calling this function, the serial console must be initialized.
///
/// \return The number of characters written.
int console_printf(const char *restrict format, ...)
    __attribute__((format(printf, 1, 2)));

/// \brief Registers the notification callback for read readiness.
///
/// \param callback The callback that will be called.
/// \param arg The argument that will be passed to \p callback.
/// \return true if the registration succeeds.
/// \return false if the registration fails because an earlier callback has not
///         been called.
bool console_notify_read_ready(void (*callback)(void *), void *arg);

/// \brief Registers the notification callback for write readiness.
///
/// \param callback The callback that will be called.
/// \param arg The argument that will be passed to \p callback.
/// \return true if the registration succeeds.
/// \return false if the registration fails because an earlier callback has not
///         been called.
bool console_notify_write_ready(void (*callback)(void *), void *arg);

/// \brief Waits until all buffered characters are sent to the serial console.
void console_flush_write_buffer(void);

/// \brief Interrupt handler. Not meant to be called directly.
void mini_uart_interrupt_handler(void);

#endif
