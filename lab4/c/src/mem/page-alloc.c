// The page frame allocator uses the buddy system. Much of the design is based
// on that described in The Art of Computer Programming by Donald Knuth,
// section 2.5.
//
// The page frame allocator maintains the page frame array, an array of size
// equal to the number of page frames and entry type `page_frame_array_entry_t`,
// which tracks each block's reservation status and order. Unlike the design
// described in the specification [spec], not all entries have valid data. If
// page i starts a block of order k, regardless of the reservation status of the
// block, the entries in the index range [i+1:i+2^k] are not read and are thus
// left uninitialized. Unlike TAOCP's design, the order of a block is stored in
// the page frame array even if the block is reserved. (Note: TAOCP's
// reservation algorithm skips storing the order of the allocated block.) This
// design decision is because, unlike TAOCP's liberation algorithm, the
// void free_pages(page_id_t) function does not have access to the order of the
// block from the arguments and must instead retrieve this information from the
// page frame array.
//
// The page frame allocator also maintains free lists of blocks, one for each
// order. Each free list is a doubly-linked list with page frame array entries
// themselves as nodes. Thus, in addition to the reservation status and the
// order of the corresponding block, each page frame array entry also contains
// the index into the page frame array of the previous and the next node on the
// free list. This implementation adopts the technique described in TAOCP
// section 2.2.5 of using the list node type itself to store the head and tail
// pointers of the list(from now on referred to as "free list header"),
// simplifying list manipulation code. TAOCP's allocator design also uses this
// technique. Since the free list headers are necessarily outside of the page
// frame array, this technique requires that the indices mentioned above be able
// to refer to page frame array entries (list nodes) outside the page frame
// array. To solve this problem without adding complexity to the code, the free
// list headers and the page frame array are allocated together, with the former
// placed right before the latter, and we use negative array indices to refer to
// entries in the free list headers.
//
// [spec]: https://oscapstone.github.io/labs/lab4.html

#include "oscos/mem/page-alloc.h"

#include "oscos/console.h"
#include "oscos/mem/memmap.h"
#include "oscos/mem/startup-alloc.h"
#include "oscos/mem/vm.h"
#include "oscos/panic.h"

// `MAX_BLOCK_ORDER` can be changed to any positive integer up to and
// including 25 without modification to the code.

typedef struct {
  bool is_avail : 1;
  unsigned order : 5;
  signed linkf : 26;
  signed linkb : 32;
} page_frame_array_entry_t;

static pa_t _pa_start;
static page_frame_array_entry_t *_page_frame_array, *_free_list;

void page_alloc_init(void) {
  const size_t n_usable_mems = mem_get_n();
  const pa_range_t *const usable_mems = mem_get();

  if (n_usable_mems == 0) {
    console_puts("WARN: page-alloc: No usable memory region.");
  }

  // Determine the starting physical address.

  _pa_start = n_usable_mems > 0 ? usable_mems[0].start : 0;

  // Check the ending physical address.

  const pa_t max_supported_pa =
      _pa_start + (1 << (PAGE_ORDER + MAX_BLOCK_ORDER));
  if (n_usable_mems > 0 &&
      usable_mems[n_usable_mems - 1].end > max_supported_pa) {
    console_printf("WARN: page-alloc: End of usable memory region 0x%" PRIxPA
                   " is greater than maximum supported PA 0x%" PRIxPA ".\n",
                   usable_mems[n_usable_mems - 1].end, max_supported_pa);
  }

  // Allocate the page frame array and the free list.

  page_frame_array_entry_t *const entries =
      startup_alloc(((MAX_BLOCK_ORDER + 1) + (1 << MAX_BLOCK_ORDER)) *
                    sizeof(page_frame_array_entry_t));
  _page_frame_array = entries + (MAX_BLOCK_ORDER + 1);
  _free_list = entries;

  // Check if the startup allocator allocated any memory the kernel shouldn't
  // use.

  if (!memmap_is_usable(
          kernel_va_range_to_pa_range(startup_alloc_get_alloc_range())))
    PANIC("page-alloc: Startup allocator allocated memory the kernel shouldn't"
          "use");

  // Initialize the page frame array. (The entire memory region is initially
  // reserved.)

  _page_frame_array[0].is_avail = false;
  _page_frame_array[0].order = MAX_BLOCK_ORDER;

  // Initialize the free list. (The free list is initially empty.)

  for (size_t order = 0; order <= MAX_BLOCK_ORDER; order++) {
    _free_list[order].linkf = _free_list[order].linkb =
        (int)order - (MAX_BLOCK_ORDER + 1);
  }

  // Mark the usable regions as usable.

  for (size_t i = 0; i < n_usable_mems; i++) {
    if (usable_mems[i].start >= max_supported_pa)
      break;

    const bool end_cut = usable_mems[i].end >= max_supported_pa;
    mark_pages(pa_range_to_page_id_range((pa_range_t){
                   .start = usable_mems[i].start,
                   .end = end_cut ? max_supported_pa : usable_mems[i].end}),
               true);
    if (end_cut)
      break;
  }

  // Mark the reserved regions as reserved.

  const size_t n_reserved_mems = reserved_mem_get_n();
  const reserved_mem_entry_t *const reserved_mems = reserved_mem_get();

  for (size_t i = 0; i < n_reserved_mems; i++) {
    if (reserved_mems[i].range.start >= max_supported_pa)
      break;

    const bool end_cut = reserved_mems[i].range.end >= max_supported_pa;
    mark_pages(
        pa_range_to_page_id_range((pa_range_t){
            .start = reserved_mems[i].range.start,
            .end = end_cut ? max_supported_pa : reserved_mems[i].range.end}),
        false);
    if (end_cut)
      break;
  }

  // Mark the region used by the startup allocator as reserved.

  mark_pages(pa_range_to_page_id_range(
                 kernel_va_range_to_pa_range(startup_alloc_get_alloc_range())),
             false);
}

/// \brief Adds a block to the free list.
///
/// \param page The page number of the first page of the block.
static void _add_block_to_free_list(const page_id_t page) {
  const size_t order = _page_frame_array[page].order;

  const int32_t free_list_first_entry = _free_list[order].linkf;
  _page_frame_array[page].linkf = free_list_first_entry;
  _page_frame_array[free_list_first_entry].linkb = page;
  _page_frame_array[page].linkb = (int)order - (MAX_BLOCK_ORDER + 1);
  _free_list[order].linkf = page;
}

/// \brief Removes a block from the free list.
///
/// \param page The page number of the first page of the block.
static void _remove_block_from_free_list(const page_id_t page) {
  _page_frame_array[_page_frame_array[page].linkb].linkf =
      _page_frame_array[page].linkf;
  _page_frame_array[_page_frame_array[page].linkf].linkb =
      _page_frame_array[page].linkb;
}

/// \brief Removes all free blocks within a page range that is valid as the page
///        range of a block from the free list.
///
/// \param range The page range. Must be valid as the page range of a block.
/// \param order The order of the block of which \p range is valid as the page
///              range. I.e., log base 2 of the span of the range.
static void
_remove_free_blocks_in_range_from_free_list(const page_id_range_t range,
                                            const size_t order) {
  if (_page_frame_array[range.start].order == order) {
    if (_page_frame_array[range.start].is_avail) {
      _remove_block_from_free_list(range.start);
    }
  } else {
    if (order == 0)
      __builtin_unreachable();

    // This will not cause integer overflow, since the maximum order is at most
    // 25 and the node ID is at most 2²⁵.
    const page_id_t mid = (range.start + range.end) / 2;

    _remove_free_blocks_in_range_from_free_list(
        (page_id_range_t){.start = range.start, .end = mid}, order - 1);
    _remove_free_blocks_in_range_from_free_list(
        (page_id_range_t){.start = mid, .end = range.end}, order - 1);
  }
}

/// \brief Splits a block.
///
/// If the block is initially free, this function updates the free lists
/// accordingly.
///
/// \param page The page number of the first page of the block.
static void _split_block(const page_id_t page) {
  if (_page_frame_array[page].order == 0)
    __builtin_unreachable();

#ifdef PAGE_ALLOC_ENABLE_LOG
  const page_id_t end_page = page + (1 << _page_frame_array[page].order);
  console_printf("DEBUG: page-alloc: Splitting block 0x%" PRIxPAGEID
                 " - 0x%" PRIxPAGEID " of order %u.\n",
                 page, end_page, _page_frame_array[page].order);
#endif

  if (_page_frame_array[page].is_avail) {
    _remove_block_from_free_list(page);
  }

  const size_t order_p1 = _page_frame_array[page].order - 1;
  const page_id_t buddy_page = page + (1 << order_p1);

  _page_frame_array[page].order = order_p1;
  _page_frame_array[buddy_page].is_avail = _page_frame_array[page].is_avail;
  _page_frame_array[buddy_page].order = order_p1;

  if (_page_frame_array[page].is_avail) {
    _add_block_to_free_list(page);
    _add_block_to_free_list(buddy_page);
  }
}

spage_id_t alloc_pages(const size_t order) {
#ifdef PAGE_ALLOC_ENABLE_LOG
  console_printf("DEBUG: page-alloc: Allocating a block of order %zu\n", order);
#endif

  // Find block.

  size_t avail_block_order;
  for (avail_block_order = order; avail_block_order <= MAX_BLOCK_ORDER;
       avail_block_order++) {
    if (_free_list[avail_block_order].linkf >=
        0) { // The free list is nonempty.
      break;
    }
  }

  if (avail_block_order >
      MAX_BLOCK_ORDER) { // No block of order >= `order` found.
    return -1;
  }

  const page_id_t page = _free_list[avail_block_order].linkf;

  // Remove from list.

  _remove_block_from_free_list(page);

  // Split.

  while (avail_block_order > order) {
    // We could have used void _split_block(page_id_t) to perform block
    // splitting, but the custom logic here avoids unnecessary free list
    // operations (adding a block to the free list and then immediately removing
    // it in the next loop iteration) that would have been performed by the said
    // function.

#ifdef PAGE_ALLOC_ENABLE_LOG
    const page_id_t end_page = page + (1 << avail_block_order);
    console_printf("DEBUG: page-alloc: Splitting block 0x%" PRIxPAGEID
                   " - 0x%" PRIxPAGEID " of order %zu.\n",
                   page, end_page, avail_block_order);
#endif

    avail_block_order--;

    const page_id_t buddy_page = page + (1 << avail_block_order);

    _page_frame_array[buddy_page].is_avail = true;
    _page_frame_array[buddy_page].order = avail_block_order;

    // Add `buddy_page` to the free list, which is empty.
    // We could have used void _add_block_to_free_list(page_id_t), but the
    // custom logic here saves a few instructions.
    _page_frame_array[buddy_page].linkf = _page_frame_array[buddy_page].linkb =
        (int)avail_block_order - (MAX_BLOCK_ORDER + 1);
    _free_list[avail_block_order].linkf = _free_list[avail_block_order].linkb =
        buddy_page;
  }

  _page_frame_array[page].is_avail = false;
  _page_frame_array[page].order = order;
  return page;
}

void free_pages(const page_id_t page) {
  const size_t order = _page_frame_array[page].order;

#ifdef PAGE_ALLOC_ENABLE_LOG
  const page_id_t end_page = page + (1 << order);
  console_printf("DEBUG: page-alloc: Freeing block 0x%" PRIxPAGEID
                 " - 0x%" PRIxPAGEID " of order %zu\n",
                 page, end_page, order);
#endif

  // Combine with buddy.

  page_id_t curr_page = page;
  size_t curr_order;
  for (curr_order = order; curr_order < MAX_BLOCK_ORDER; curr_order++) {
    const page_id_t buddy_page = curr_page ^ (1 << curr_order);
    if (!(_page_frame_array[buddy_page].is_avail &&
          _page_frame_array[buddy_page].order == curr_order))
      break;

#ifdef PAGE_ALLOC_ENABLE_LOG
    const page_id_t end_curr_page = curr_page + (1 << curr_order);
    const page_id_t end_buddy_page = buddy_page + (1 << curr_order);
    console_printf(
        "DEBUG: page-alloc: Combining block 0x%" PRIxPAGEID " - 0x%" PRIxPAGEID
        " of order %zu with its buddy 0x%" PRIxPAGEID " - 0x%" PRIxPAGEID ".\n",
        curr_page, end_curr_page, curr_order, buddy_page, end_buddy_page);
#endif

    _remove_block_from_free_list(buddy_page);
    if (buddy_page < curr_page) {
      curr_page = buddy_page;
    }
  }

  _page_frame_array[curr_page].is_avail = true;
  _page_frame_array[curr_page].order = curr_order;
  _add_block_to_free_list(curr_page);
}

static void _mark_pages_rec(const page_id_range_t range, const bool is_avail,
                            const size_t order,
                            const page_id_range_t block_range) {
  if (_page_frame_array[block_range.start].order == order &&
      _page_frame_array[block_range.start].is_avail ==
          is_avail) { // The entire block is already marked as the desired
                      // reservation status.
    // No-op.
  } else if (range.start == block_range.start && range.end == block_range.end) {
    // Mark the entire block.

    _remove_free_blocks_in_range_from_free_list(range, order);

    _page_frame_array[range.start].is_avail = is_avail;
    _page_frame_array[range.start].order = order;
    if (is_avail) {
      _add_block_to_free_list(range.start);
    }
  } else { // Recursive case.
    if (order == 0)
      __builtin_unreachable();

    // Split the block if needed.
    if (_page_frame_array[block_range.start].order == order) {
      _split_block(block_range.start);
    }

    // This will not cause integer overflow, since the maximum order is at most
    // 25 and the node ID is at most 2²⁵.
    const size_t mid = (block_range.start + block_range.end) / 2;

    if (range.end <= mid) { // The range lies entirely within the left half of
                            // the node range.
      _mark_pages_rec(
          range, is_avail, order - 1,
          (page_id_range_t){.start = block_range.start, .end = mid});
    } else if (mid <= range.start) { // The range lies entirely within the right
                                     // half of the node range.
      _mark_pages_rec(range, is_avail, order - 1,
                      (page_id_range_t){.start = mid, .end = block_range.end});
    } else { // The range crosses the middle of the node range.
      _mark_pages_rec(
          (page_id_range_t){.start = range.start, .end = mid}, is_avail,
          order - 1, (page_id_range_t){.start = block_range.start, .end = mid});
      _mark_pages_rec((page_id_range_t){.start = mid, .end = range.end},
                      is_avail, order - 1,
                      (page_id_range_t){.start = mid, .end = block_range.end});
    }
  }
}

void mark_pages(const page_id_range_t range, const bool is_avail) {
#ifdef PAGE_ALLOC_ENABLE_LOG
  console_printf("DEBUG: page-alloc: Marking pages 0x%" PRIxPAGEID
                 " - 0x%" PRIxPAGEID " as %s.\n",
                 range.start, range.end, is_avail ? "available" : "reserved");
#endif

  // TODO: Switch to a non-recursive implementation.
  _mark_pages_rec(range, is_avail, MAX_BLOCK_ORDER,
                  (page_id_range_t){.start = 0, .end = 1 << MAX_BLOCK_ORDER});
}

pa_t page_id_to_pa(const page_id_t page) {
  return _pa_start + (page << PAGE_ORDER);
}

page_id_range_t pa_range_to_page_id_range(const pa_range_t range) {
  return (page_id_range_t){
      .start = (range.start - _pa_start) >> PAGE_ORDER,
      .end = ((range.end - _pa_start) + ((1 << PAGE_ORDER) - 1)) >> PAGE_ORDER};
}
