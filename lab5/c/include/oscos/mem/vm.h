#ifndef OSCOS_MEM_VM_H
#define OSCOS_MEM_VM_H

#include "oscos/mem/types.h"

/// \brief Converts a kernel space virtual address into its corresponding
///        physical address.
pa_t kernel_va_to_pa(const void *va) __attribute__((const));

/// \brief Converts a physical address into its corresponding kernel space
///        virtual address.
void *pa_to_kernel_va(pa_t pa) __attribute__((const));

/// \brief Converts a kernel space virtual address range into its corresponding
///        physical address range.
pa_range_t kernel_va_range_to_pa_range(va_range_t range) __attribute__((const));

#endif
