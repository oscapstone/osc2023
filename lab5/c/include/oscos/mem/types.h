#ifndef OSCOS_MEM_TYPES_H
#define OSCOS_MEM_TYPES_H

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

#endif
