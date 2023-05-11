#ifndef OSCOS_MEM_SIMPLE_ALLOC_H
#define OSCOS_MEM_SIMPLE_ALLOC_H

#include <stdalign.h>
#include <stddef.h>

/// \brief Dynamically allocates memory.
///
/// Note that memory allocated by this function cannot be freed.
///
/// \param size The requested size in bytes of the allocation.
/// \return The pointer to the allocated memory, or NULL if the request cannot
///         be fulfilled.
void *simple_alloc(size_t size)
    __attribute__((alloc_size(1), assume_aligned(alignof(max_align_t)),
                   malloc));

#endif
