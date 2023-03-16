#ifndef OSCOS_SIMPLE_MALLOC_H
#define OSCOS_SIMPLE_MALLOC_H

#include <stddef.h>

/// \brief Dynamically allocates memory.
///
/// Note that memory allocated by this function cannot be freed. Also, this
/// function is not thread-safe.
///
/// \param size The requested size in bytes of the allocation.
/// \return The pointer to the allocated memory, or NULL if the request cannot
///         be fulfilled.
void *simple_malloc(size_t size);

#endif
