/// \file include/oscos/mem/startup-alloc.h
/// \brief Startup allocator.
///
/// The startup allocator is a simple bump pointer allocator that is usable even
/// when no other subsystems are initialized. This memory allocator allocates
/// memory starting from the end of the kernel. Note that memory allocated using
/// this memory allocator cannot be freed.

#ifndef OSCOS_MEM_STARTUP_ALLOC_H
#define OSCOS_MEM_STARTUP_ALLOC_H

#include <stdalign.h>
#include <stddef.h>

#include "oscos/mem/types.h"

/// \brief Initializes the startup allocator.
void startup_alloc_init(void);

/// \brief Dynamically allocates memory using the startup allocator.
///
/// Note that memory allocated by this function cannot be freed.
///
/// \param size The requested size in bytes of the allocation.
/// \return The pointer to the allocated memory, or NULL if the request cannot
///         be fulfilled.
void *startup_alloc(size_t size)
    __attribute__((alloc_size(1), assume_aligned(alignof(max_align_t)),
                   malloc));

/// \brief Gets the memory range allocated through the startup allocator.
va_range_t startup_alloc_get_alloc_range(void) __attribute__((pure));

#endif
