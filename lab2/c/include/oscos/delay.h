#ifndef OSCOS_DELAY_H
#define OSCOS_DELAY_H

#include <stdint.h>

#define NS_PER_SEC ((uint64_t)1000000000)

/// \brief Delays for the specified number of nanoseconds.
///
/// Note that this function may delay for a longer period than specified.
void delay_ns(uint64_t ns);

#endif
