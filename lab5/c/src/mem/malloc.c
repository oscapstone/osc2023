// The design of the dynamic memory allocator resembles that of the slab
// allocator to some extent. However, compared to the slab allocator
// implementation in the Linux kernel [linux-slab-alloc], it only achieves the
// first of the three principle aims, namely, to help eliminate internal
// fragmentation.
//
// Each slab is backed by a single page. The first 32 bytes of the page are
// reserved for bookkeeping data, while the remaining area is split into
// equally-sized chunks that are units of allocation. There are different kinds
// of slabs for many different slot sizes. This allocator maintains free lists
// of slabs, one for each kind, chaining slabs of the same kind and with at
// least one available slot together.
//
// When the last reserved slot of a slab becomes available, the slab is
// immediately destroyed and the underlying page is returned to the page frame
// allocator. This design causes thrashing on certain allocation/deallocation
// patterns, but it keeps the code simple.
//
// Large allocation requests bypass the slab allocator and goes directly to the
// page frame allocator. The allocated memory is appropriately tagged so that
// void free(void *ptr) knows which memory allocator a memory is allocated with.
// An allocation request is considered large if the size is greater than 126
// `max_align_t`, the maximum slot size that allows a slab to hold at least two
// slots. Indeed, if the request size is so large that a slab able to satisfy
// the request can only hold a single slot, then using the slab allocator offers
// no advantage at all. If the request size is even larger, then a slab with a
// large enough slot size will not be able to hold even a single slot.
//
// [linux-slab-alloc]:
//   https://www.kernel.org/doc/gorman/html/understand/understand011.html

#include "oscos/mem/malloc.h"

#include <stdalign.h>
#include <stdint.h>

#include "oscos/libc/string.h"
#include "oscos/mem/page-alloc.h"
#include "oscos/mem/vm.h"
#include "oscos/utils/math.h"

/// \brief A node of a doubly-linked list.
typedef struct list_node_t {
  struct list_node_t *prev, *next;
} list_node_t;

/// \brief Slab metadata.
typedef struct {
  /// \brief The number of slots.
  uint8_t n_slots;
  /// \brief The size of each slot in numbers of `max_align_t`.
  uint8_t slot_size;
} slab_metadata_t;

/// \brief Slab (or not).
typedef struct {
  /// \brief Node of the free list.
  ///
  /// If `free_list_node.prev` is NULL, then this "slab" is in fact not a slab
  /// but a memory allocated for a large allocation request.
  ///
  /// This field is put in the first position, so that obtaining a slab_t * from
  /// a pointer to its `free_list_node` field is a no-op.
  list_node_t free_list_node;
  /// \brief Pointer to the head of the free list of the slabs of the same kind.
  ///
  /// This is used when the slab adds itself to the free list.
  list_node_t *free_list_head;
  /// \brief Metadata.
  slab_metadata_t metadata;
  /// \brief The number of reserved slots.
  uint8_t n_slots_reserved;
  /// \brief Bitset of reserved slots.
  ///
  /// The jth bit of `slots_reserved_bitset[i]` if set iff the i*64 + j slot is
  /// reserved.
  uint64_t slots_reserved_bitset[4];
  /// \brief The memory for the slots.
  alignas(alignof(max_align_t)) unsigned char slots[];
} slab_t;

/// \brief Metadata of all supported types of slabs.
static const slab_metadata_t SLAB_METADATA[] = {
    {.n_slots = 252, .slot_size = 1}, {.n_slots = 126, .slot_size = 2},
    {.n_slots = 84, .slot_size = 3},  {.n_slots = 63, .slot_size = 4},
    {.n_slots = 50, .slot_size = 5},  {.n_slots = 42, .slot_size = 6},
    {.n_slots = 36, .slot_size = 7},  {.n_slots = 31, .slot_size = 8},
    {.n_slots = 28, .slot_size = 9},  {.n_slots = 25, .slot_size = 10},
    {.n_slots = 22, .slot_size = 11}, {.n_slots = 21, .slot_size = 12},
    {.n_slots = 19, .slot_size = 13}, {.n_slots = 18, .slot_size = 14},
    {.n_slots = 16, .slot_size = 15}, {.n_slots = 15, .slot_size = 16},
    {.n_slots = 14, .slot_size = 18}, {.n_slots = 13, .slot_size = 19},
    {.n_slots = 12, .slot_size = 21}, {.n_slots = 11, .slot_size = 22},
    {.n_slots = 10, .slot_size = 25}, {.n_slots = 9, .slot_size = 28},
    {.n_slots = 8, .slot_size = 31},  {.n_slots = 7, .slot_size = 36},
    {.n_slots = 6, .slot_size = 42},  {.n_slots = 5, .slot_size = 50},
    {.n_slots = 4, .slot_size = 63},  {.n_slots = 3, .slot_size = 84},
    {.n_slots = 2, .slot_size = 126}};

/// \brief The number of slab types.
#define N_SLAB_TYPES (sizeof(SLAB_METADATA) / sizeof(slab_metadata_t))

/// \brief The threshold in numbers of `max_align_t` an allocation request whose
///        size is more than which is considered a large allocation request.
#define LARGE_ALLOC_THRESHOLD 126

/// \brief Free lists of slabs for each slab type.
static list_node_t _slab_free_lists[N_SLAB_TYPES];

/// \brief Gets the slab type ID (the index that can be used to index
///        `SLAB_METADATA` or the free list) from the size of the allocation
///        request.
///
/// \param n_units The size of the allocation request in numbers of "allocation
///                units", i.e., `max_align_t`.
static size_t _get_slab_type_id(const size_t n_units) {
  if (n_units == 0 || n_units > LARGE_ALLOC_THRESHOLD)
    __builtin_unreachable();

  return n_units <= 16 ? n_units - 1 : N_SLAB_TYPES + 1 - 252 / n_units;
}

// Slab operations.

/// \brief Adds a slab to its free list.
///
/// The `free_list_head` field of \p slab must be initialized and \p slab must
/// not have been on any free list.
static void _add_slab_to_free_list(slab_t *const slab) {
  list_node_t *const free_list_first_entry = slab->free_list_head->next;
  slab->free_list_node.next = free_list_first_entry;
  free_list_first_entry->prev = &slab->free_list_node;
  slab->free_list_node.prev = slab->free_list_head;
  slab->free_list_head->next = &slab->free_list_node;
}

/// \brief Removes a slab from its free list.
///
/// \p slab must have been on a free list.
static void _remove_slab_from_free_list(slab_t *const slab) {
  slab->free_list_node.prev->next = slab->free_list_node.next;
  slab->free_list_node.next->prev = slab->free_list_node.prev;
}

/// \brief Allocates a new slab and adds it onto its free list.
static slab_t *_alloc_slab(const size_t slab_type_id) {
  // Allocate space for the slab.

  const spage_id_t page = alloc_pages(0);
  if (page < 0)
    return NULL;

  slab_t *slab = (slab_t *)pa_to_kernel_va(page_id_to_pa(page));
  if (!slab) {
    // We cannot accept a null slab pointer in this implementation due to the
    // `free_list_node.prev` being used as a tag to identify memories for large
    // allocation requests. Allocate a new page and return the previously
    // allocated page to the page frame allocator.
    // (In practice, this code path is never taken.)

    const spage_id_t another_page = alloc_pages(0);
    free_pages(page);
    if (another_page < 0) {
      return NULL;
    }

    slab = (slab_t *)pa_to_kernel_va(page_id_to_pa(another_page));
  }

  // Initialize the fields.

  slab->free_list_head = &_slab_free_lists[slab_type_id];
  slab->metadata = SLAB_METADATA[slab_type_id];
  slab->n_slots_reserved = 0;
  memset(slab->slots_reserved_bitset, 0, sizeof(slab->slots_reserved_bitset));
  _add_slab_to_free_list(slab);

  return slab;
}

/// \brief Gets a slab of the given type with at least one free slot. If there
///        is none, allocates a new one.
static slab_t *_get_or_alloc_slab(const size_t slab_type_id) {
  if (_slab_free_lists[slab_type_id].next ==
      &_slab_free_lists[slab_type_id]) { // The free list is empty.
    return _alloc_slab(slab_type_id);
  } else {
    list_node_t *const free_list_first_entry =
        _slab_free_lists[slab_type_id].next;
    return (slab_t *)((char *)free_list_first_entry -
                      offsetof(slab_t, free_list_node));
  }
}

/// \brief Gets the index of the first free slot of the given slab.
///
/// \p slab must have at least one free slot.
static size_t _get_first_free_slot_ix(const slab_t *const slab) {
  for (size_t i = 0;; i++) {
    uint64_t j;
    __asm__ __volatile__("clz %0, %1"
                         : "=r"(j)
                         : "r"(~slab->slots_reserved_bitset[i]));
    if (j !=
        64) { // The (63-j)th bit of `slab->slots_reserved_bitset[i]` is clear.
      return i * 64 + (63 - j);
    }
  }
}

/// \brief Allocates a slot from the given slab.
///
/// \p slab must have at least one free slot.
static void *_alloc_from_slab(slab_t *const slab) {
  const size_t free_slot_ix = _get_first_free_slot_ix(slab);

  // Mark the `free_slot_ix`th slot as reserved.

  slab->n_slots_reserved++;
  slab->slots_reserved_bitset[free_slot_ix / 64] |= 1 << (free_slot_ix % 64);

  // Remove itself from its free list if there are no free slots.

  if (slab->n_slots_reserved == slab->metadata.n_slots) {
    _remove_slab_from_free_list(slab);
  }

  return slab->slots + free_slot_ix * (slab->metadata.slot_size * 16);
}

/// \brief Frees a slot to the given slab.
///
/// \param slab The slab.
/// \param ptr The pointer to the slot.
static void _free_to_slab(slab_t *const slab, void *const ptr) {
  const size_t slot_ix =
      ((uintptr_t)ptr - (uintptr_t)slab) / (slab->metadata.slot_size * 16);

  // Adds the slab to its free list if it wasn't on its free list.

  if (slab->n_slots_reserved == slab->metadata.n_slots) {
    _add_slab_to_free_list(slab);
  }

  // Mark the slot as available.

  slab->n_slots_reserved--;
  slab->slots_reserved_bitset[slot_ix / 64] &= ~(1 << (slot_ix % 64));

  // Return the slab to the page frame allocator if it has no reserved slots.

  if (slab->n_slots_reserved == 0) {
    _remove_slab_from_free_list(slab);
    free_pages(pa_to_page_id(kernel_va_to_pa(slab)));
  }
}

// Large allocation.

/// \brief Allocates memory using the large allocation mechanism.
///
/// In principle, using this function to satisfy small allocation requests will
/// not cause problems (UB or kernel panic), but doing so wastes a lot of
/// memory.
///
/// \param size The size of the allocation in bytes.
static void *_malloc_large(const size_t size) {
  const size_t actual_size = alignof(max_align_t) + size;
  const size_t n_pages = (actual_size + ((1 << PAGE_ORDER) - 1)) >> PAGE_ORDER;
  const size_t block_order = clog2(n_pages);

  spage_id_t page = alloc_pages(block_order);
  if (page < 0)
    return NULL;

  char *page_va = pa_to_kernel_va(page_id_to_pa(page));
  // Mark the "slab" as not a slab.
  ((slab_t *)page_va)->free_list_node.prev = NULL;

  return page_va + alignof(max_align_t);
}

/// \brief Frees memory allocated using void *_malloc_large(size_t).
/// \param slab_ptr The pointer to the "slab". (N.B. not the pointer returned by
///                 void *_malloc_large(size_t)!)
static void _free_large(slab_t *const slab_ptr) {
  free_pages(pa_to_page_id(kernel_va_to_pa(slab_ptr)));
}

// Public functions.

void malloc_init(void) {
  // Initialize the free lists. (All free lists are initially empty.)

  for (size_t i = 0; i < N_SLAB_TYPES; i++) {
    _slab_free_lists[i].prev = _slab_free_lists[i].next = &_slab_free_lists[i];
  }
}

void *malloc(const size_t size) {
  if (size == 0)
    return NULL;

  const size_t n_units = (size + (alignof(max_align_t) - 1)) >> 4;
  if (n_units > LARGE_ALLOC_THRESHOLD)
    return _malloc_large(size);

  const size_t slab_type_id = _get_slab_type_id(n_units);
  slab_t *const slab = _get_or_alloc_slab(slab_type_id);
  if (!slab)
    return NULL;

  return _alloc_from_slab(slab);
}

void free(void *const ptr) {
  if (!ptr)
    return;

  slab_t *const ptr_s = (slab_t *)((uintptr_t)ptr & ~((1 << PAGE_ORDER) - 1));
  if (!ptr_s->free_list_node.prev) { // Not a slab.
    _free_large(ptr_s);
  } else {
    _free_to_slab(ptr_s, ptr);
  }
}
