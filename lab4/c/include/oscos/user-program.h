#ifndef OSCOS_USER_PROGRAM_H
#define OSCOS_USER_PROGRAM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdnoreturn.h>

/// \brief Loads the user program from somewhere in the memory to the place in
///        memory where it'll be executed.
///
/// \param start The starting address of where the user program is currently
///              located. (Do not confuse it with the entry point of the user
///              program. The latter is hardcoded in the kernel.)
/// \param len The length of the user program.
/// \return true if the loading succeeds.
/// \return false if the loading failed due to the user program being too long.
bool load_user_program(const void *start, size_t len);

/// \brief Runs the loaded user program.
noreturn void run_user_program(void);

#endif
