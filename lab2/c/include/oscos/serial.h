/// \file include/oscos/serial.h
/// \brief Serial console.
///
/// The serial console reads and writes data to the mini UART.
///
/// Before using the serial console, it must be initialized by calling
/// serial_init(void) exactly once. Initializing the serial console twice or
/// operating on the serial console before initialization are not checked and
/// may have unintended consequences.
///
/// In order to prevent race condition, the serial console is protected by a
/// spinlock. If the serial console may be accessed from multiple threads, the
/// current thread must acquire the lock by calling serial_lock(void) before
/// operating the serial console (except initialization). The lock is released
/// by calling serial_unlock(void).
///
/// The serial console implements two modes: text mode and binary mode. In text
/// mode, automatic newline translation is performed. A "\r" character received
/// by the mini UART will be read as "\n", and writing "\n" to the serial
/// console will send "\r\n" down the mini UART. In binary mode, automatic
/// newline translation is not performed. Any byte received by the mini UART
/// will be read as-is, and every character written to the serial console will
/// also be sent as-is. Upon initialization, the mode is set to text mode.

#ifndef OSCOS_SERIAL_H
#define OSCOS_SERIAL_H

#include <stddef.h>
#include <stdint.h>

/// \brief The mode of the serial console.
typedef enum {
  SM_TEXT,  ///< Text mode. Performs newline translation.
  SM_BINARY ///< Binary mode. Every byte are sent/received as-is.
} SerialMode;

/// \brief Initializes the serial console.
///
/// See the file-level documentation for requirements on initialization.
void serial_init(void);

/// \brief De-initializes the serial console.
///
/// When calling this function, the serial console must be initialized. This
/// function may not be called when the serial console is being accessed by any
/// other thread.
void serial_deinit(void);

/// \brief Acquires the lock of the serial console.
///
/// When calling this function, the current thread must not have acquired the
/// lock. Otherwise, the function hangs indefinitely. There is no recursive
/// locking support.
void serial_lock(void);

/// \brief Releases the lock of the serial console.
///
/// When calling this function, the current thread must have acquired the lock.
/// Otherwise, the lock may become corrupted and race conditions may occur on
/// later accesses to the serial console.
void serial_unlock(void);

/// \brief Sets the mode of the serial console.
///
/// When calling this function in a multi-threaded context, the current thread
/// must have acquired the lock of the serial console.
///
/// \param mode The mode. Must be `SM_TEXT` or `SM_BINARY`.
void serial_set_mode(SerialMode mode);

/// \brief Waits until all characters in the transmit FIFO have been sent.
///
/// When calling this function, the serial console must be initialized. Also, in
/// a multi-threaded context, the current thread must have acquired the lock of
/// the serial console.
void serial_flush(void);

/// \brief Reads a character from the serial console.
///
/// When calling this function, the serial console must be initialized. Also, in
/// a multi-threaded context, the current thread must have acquired the lock of
/// the serial console.
char serial_getc(void);

/// \brief Writes a character to the serial console.
///
/// When calling this function, the serial console must be initialized. Also, in
/// a multi-threaded context, the current thread must have acquired the lock of
/// the serial console.
char serial_putc(char c);

/// \brief Writes a string to the serial console without trailing newline.
///
/// When calling this function, the serial console must be initialized. Also, in
/// a multi-threaded context, the current thread must have acquired the lock of
/// the serial console.
void serial_fputs(const char *s);

/// \brief Writes a string to the serial console with trailing newline.
///
/// When calling this function, the serial console must be initialized. Also, in
/// a multi-threaded context, the current thread must have acquired the lock of
/// the serial console.
void serial_puts(const char *s);

/// \brief Writes a `uint32_t` to the serial console in hexadecimal.
///
/// When calling this function, the serial console must be initialized. Also, in
/// a multi-threaded context, the current thread must have acquired the lock of
/// the serial console.
void serial_print_hex(uint32_t x);

#endif
