/// \file include/oscos/mem/malloc.h
/// \brief Dynamic memory allocator.
///
/// The dynamic memory allocator is a general-purpose memory allocator that
/// allocates physically-contiguous memory.

#ifndef OSCOS_MEM_MALLOC_H
#define OSCOS_MEM_MALLOC_H

#include <stdalign.h>
#include <stddef.h>

/// \brief Initializes the dynamic memory allocator.
void malloc_init(void);

/// \brief Frees memory allocated using the dynamic memory allocator.
///
/// \param ptr The pointer to the allocated memory.
void free(void *ptr);

/// \brief Dynamically allocates memory using the dynamic memory allocator.
///
/// \param size The requested size in bytes of the allocation.
/// \return The pointer to the allocated memory, or NULL if the request cannot
///         be fulfilled.
void *malloc(size_t size)
    __attribute__((alloc_size(1), assume_aligned(alignof(max_align_t)), malloc,
                   malloc(free, 1)));

#endif
