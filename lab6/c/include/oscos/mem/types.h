#ifndef OSCOS_MEM_TYPES_H
#define OSCOS_MEM_TYPES_H

#include <stddef.h>

#include "oscos/libc/inttypes.h"

/// \brief Physical address.
typedef uint32_t pa_t;

/// \brief Maximum value of pa_t.
#define PA_MAX UINT32_MAX

/// \brief Format specifier for printing a pa_t in lowercase hexadecimal format.
#define PRIxPA PRIx32

/// \brief Page ID.
typedef uint32_t page_id_t;

/// \brief Format specifier for printing a page_id_t in lowercase hexadecimal
///        format.
#define PRIxPAGEID PRIx32

/// \brief Signed page ID.
typedef int32_t spage_id_t;

/// \brief Physical address range.
typedef struct {
  pa_t start, end;
} pa_range_t;

/// \brief Page range.
typedef struct {
  page_id_t start, end;
} page_id_range_t;

/// \brief Virtual address range.
typedef struct {
  void *start, *end;
} va_range_t;

/// \brief Shared, reference-counted page.
typedef struct {
  size_t ref_count;
  page_id_t page_id;
} shared_page_t;

/// \brief Initializes a shared page.
///
/// This function sets the reference count of the newly-created shared page
/// to 1.
shared_page_t *shared_page_init(page_id_t page_id);

/// \brief Clones a shared page.
///
/// This function increases the reference count of the shared page by 1.
///
/// \param shared_page The shared page to clone.
/// \return \p shared_page
shared_page_t *shared_page_clone(shared_page_t *shared_page);

/// \brief Drops a shared page.
///
/// This function decreases the reference count of the shared page by 1. The
/// shared page structure and the underlying page will be freed when the
/// reference count reaches 0.
void shared_page_drop(shared_page_t *shared_page);

#endif
