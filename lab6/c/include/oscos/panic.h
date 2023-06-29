#ifndef OSCOS_PANIC_H
#define OSCOS_PANIC_H

#include <stdnoreturn.h>

/// \brief Starts a kernel panic.
#define PANIC(...) panic_begin(__FILE__, __LINE__, __VA_ARGS__)

/// \brief Starts a kernel panic.
///
/// In most cases, the PANIC macro should be used instead of directly calling
/// this function.
///
/// \param file The path of the source file to appear in the panic message.
/// \param line The line number to appear in the panic message.
/// \param format The format string of the panic message.
///
/// \see PANIC
noreturn void panic_begin(const char *restrict file, int line,
                          const char *restrict format, ...)
    __attribute__((cold, format(printf, 3, 4)));

#endif
