/// \file include/oscos/mem/memmap.h
/// \brief Memory map.
///
/// The memory map contains information about which part of the memory is usable
/// and which part of the memory is reserved.

#ifndef OSCOS_MEM_MEMMAP_H
#define OSCOS_MEM_MEMMAP_H

#include <stdbool.h>
#include <stddef.h>

#include "oscos/mem/types.h"

/// \brief Purpose of memory reservation.
typedef enum {
  RMP_FIRMWARE, ///< Reserved by firmware for whatever reason.
  RMP_KERNEL,   ///< Kernel text, rodata, data, and bss.
  RMP_INITRD,   ///< Initial ramdisk.
  RMP_DTB,      ///< Devicetree blob.
  RMP_STACK,    ///< Kernel stack.
  RMP_UNKNOWN   ///< Unknown reason.
} reserved_mem_purpose_t;

/// \brief A reserved memory entry in the memory map.
typedef struct {
  pa_range_t range;
  reserved_mem_purpose_t purpose;
} reserved_mem_entry_t;

/// \brief Initializes the memory map.
void memmap_init(void);

/// \brief Adds a usable memory region.
///
/// \return true if the addition succeeds.
/// \return false if the addition fails because the usable memory region array
///         is out of free space.
bool mem_add(pa_range_t mem);

/// \brief Sorts the usable memory regions in ascending order.
void mem_sort(void);

/// \brief Gets the number of usable memory regions.
size_t mem_get_n(void);

/// \brief Gets the pointer to the usable memory region array.
const pa_range_t *mem_get(void);

/// \brief Adds a reserved memory region.
///
/// \return true if the addition succeeds.
/// \return false if the addition fails because the reserved memory region array
///         is out of free space.
bool reserved_mem_add(reserved_mem_entry_t reserved_mem);

/// \brief Sorts the reserved memory regions in ascending order.
void reserved_mem_sort(void);

/// \brief Gets the number of reserved memory regions.
size_t reserved_mem_get_n(void);

/// \brief Gets the pointer to the reserved memory region array.
const reserved_mem_entry_t *reserved_mem_get(void);

/// \brief Scans the device memory and adds the results to the memory map.
///
/// This function starts a kernel panic when either the usable memory region
/// array or the reserved memory region array runs out of space.
///
/// This function sorts the usable and reserved memory regions in ascending
/// order as if by calling void mem_sort(void) and void reserved_mem_sort(void).
void scan_mem(void);

/// \brief Checks whether or not the memory map is well-formed.
///
/// A memory map is well-formed if all of the following conditions hold:
/// - All usable memory regions do not overlap.
/// - All reserved memory regions do not overlap and are within one of the
///   usable memory regions.
///
/// Before calling this function, the memory regions must be sorted in ascending
/// order (automatically done by void scan_mem(void)).
bool check_memmap(void);

/// \brief Prints the device memory map to the serial console.
///
/// Before calling this function, the usable memory regions and the reserved
/// memory regions must be sorted in ascending order (automatically done by
/// void scan_mem(void)).
void print_memmap(void);

/// \brief Checks whether or not a memory range is usable, i.e., within a usable
///        memory region but does not overlap with any reserved memory regions.
///
/// The memory map must be well-formed when calling this function.
bool memmap_is_usable(pa_range_t range);

#endif
