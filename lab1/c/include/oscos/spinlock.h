// Currently doesn't work on a real Raspberry Pi Model 3 B+.
// TODO: Fix.
// * FIXME: After fixing the lock, re-enable serial console locking in
// * `src/serial.c`.

#ifndef OSCOS_SPINLOCK_H
#define OSCOS_SPINLOCK_H

#include <stdatomic.h>

/// \brief Acquires a spinlock.
///
/// \param lock The spinlock. Must be of type `volatile? atomic_flag *`.
#define SPIN_LOCK(lock)                                                        \
  do {                                                                         \
    while (atomic_flag_test_and_set_explicit(lock, memory_order_acquire))      \
      ;                                                                        \
  } while (0)

/// \brief Releases a spinlock.
///
/// \param lock The spinlock. Must be of type `volatile? atomic_flag *`.
#define SPIN_UNLOCK(lock) atomic_flag_clear_explicit(lock, memory_order_release)

#endif
