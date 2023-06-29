#ifndef OSCOS_TIMER_DELAY_H
#define OSCOS_TIMER_DELAY_H

#include <stdint.h>

/// \brief Delays for the specified number of nanoseconds.
///
/// Note that this function may delay for a longer period than specified.
void delay_ns(uint64_t ns);

#endif
