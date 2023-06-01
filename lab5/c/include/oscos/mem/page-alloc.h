/// \file include/oscos/mem/page-alloc.h
/// \brief Page frame allocator.

#ifndef OSCOS_MEM_PAGE_ALLOC_H
#define OSCOS_MEM_PAGE_ALLOC_H

#include <stdbool.h>
#include <stddef.h>

#include "oscos/mem/types.h"

#define PAGE_ORDER 12
#define MAX_BLOCK_ORDER 18

/// \brief Initializes the page frame allocator.
///
/// After calling this function, the startup allocator should not be used.
void page_alloc_init(void);

/// \brief Allocates a block of page frames.
/// \param order The order of the block.
/// \return The page number of the first page, or a negative number if the
///         request cannot be fulfilled.
spage_id_t alloc_pages(size_t order);

/// \brief Frees a block of page frames.
/// \param page The page number of the first page of the block.
void free_pages(page_id_t page);

/// \brief Marks a contiguous range of page frames as either reserved or
///        available.
///
/// Note that the range can be arbitrary and doesn't need to be a block.
///
/// \param range The range of page frames to mark.
/// \param is_avail The target reservation status.
void mark_pages(page_id_range_t range, bool is_avail);

/// \brief Converts the given page ID into its corresponding physical address.
pa_t page_id_to_pa(page_id_t page) __attribute__((pure));

/// \brief Converts the given physical address into its corresponding page ID.
///
/// Before conversion, the physical address is rounded down to the page
/// boundary. I.e., the returned page ID is the page ID whose corresponding
/// physical address range covers the given physical address.
page_id_t pa_to_page_id(pa_t pa) __attribute__((pure));

/// \brief Converts the given physical address range into its corresponding page
///        ID range.
///
/// Before conversion, the starting physical address is rounded down to the page
/// boundary and the ending physical address is rounded up to the page boundary.
/// I.e., the returned page ID range is the smallest page ID range whose
/// corresponding physical address range covers the given physical address
/// range.
page_id_range_t pa_range_to_page_id_range(pa_range_t range)
    __attribute__((pure));

#endif
