#include "oscos/libc/stdlib.h"

#include "oscos/libc/string.h"

#define INSERTION_SORT_THRESHOLD 16

// Insertion sort.

/// \brief Insertion sort.
static void
_insertion_sort(void *const base, const size_t nmemb, const size_t size,
                int (*const compar)(const void *, const void *, void *),
                void *const arg) {
  for (size_t i = 1; i < nmemb; i++) {
    for (size_t j = i; j > 0; j--) {
      void *const pl = (char *)base + (j - 1) * size, *const pr =
                                                          (char *)pl + size;
      if (compar(pl, pr, arg) < 0)
        break;
      memswp(pl, pr, size);
    }
  }
}

// Quicksort.

/// \brief The result of partitioning, indicating the pivot points.
typedef struct {
  size_t eq_start; ///< The starting index of the part where the element
                   ///< compares equal to the pivot.
  size_t gt_start; ///< The starting index of the part where the element
                   ///< compares greater than the pivot.
} partition_result_t;

/// \brief Median-of-3 pivot selection.
///
/// \return The index of the chosen pivot.
static size_t
_select_pivot(const void *const base, const size_t nmemb, const size_t size,
              int (*const compar)(const void *, const void *, void *),
              void *const arg) {
  if (nmemb == 0)
    __builtin_unreachable();
  if (nmemb < 3)
    return 0;

  const size_t imid = nmemb / 2, ilast = nmemb - 1;
  const void *const mid = (char *)base + imid * size, *const last =
                                                          (char *)base +
                                                          ilast * size;
  return compar(base, mid, arg) <= 0    ? compar(mid, last, arg) <= 0    ? imid
                                          : compar(base, last, arg) <= 0 ? ilast
                                                                         : 0
         : compar(mid, last, arg) > 0   ? imid
         : compar(base, last, arg) <= 0 ? 0
                                        : ilast;
}

/// \brief Three-way partitioning, using the first element as the pivot.
static partition_result_t
_partition(void *const base, const size_t nmemb, const size_t size,
           int (*const compar)(const void *, const void *, void *),
           void *const arg) {
  if (nmemb == 0)
    __builtin_unreachable();

  // Partition every element except for the first one (pivot).

  size_t il = 1, im = 1, ir = nmemb;
  while (im < ir) {
    void *const pl = (char *)base + il * size,
                *const pm = (char *)base + im * size,
                *const prm1 = (char *)base + (ir - 1) * size;

    const int compar_result = compar(pm, base, arg);
    if (compar_result < 0) {
      memswp(pl, pm, size);
      il++;
      im++;
    } else if (compar_result > 0) {
      memswp(pm, prm1, size);
      ir--;
    } else {
      im++;
    }
  }

  // Move the pivot to its place.

  if (il != 0) {
    memswp(base, (char *)base + --il * size, size);
  }

  return (partition_result_t){.eq_start = il, .gt_start = im};
}

/// \brief Quicksort.
///
/// A basic quicksort with median-of-3 pivot selection, three-way partitioning,
/// and insertion sort for small arrays.
static void _quicksort(void *const base, const size_t nmemb, const size_t size,
                       int (*const compar)(const void *, const void *, void *),
                       void *const arg) {
  if (nmemb <= INSERTION_SORT_THRESHOLD) {
    _insertion_sort(base, nmemb, size, compar, arg);
    return;
  }

  // Select the pivot.

  const size_t pivot_ix = _select_pivot(base, nmemb, size, compar, arg);

  // Partition the array.

  if (pivot_ix != 0) {
    memswp(base, (char *)base + pivot_ix * size, size);
  }

  const partition_result_t partition_result =
      _partition(base, nmemb, size, compar, arg);

  // Recursively sort the subarrays.

  _quicksort(base, partition_result.eq_start, size, compar, arg);
  _quicksort((char *)base + partition_result.gt_start * size,
             nmemb - partition_result.gt_start, size, compar, arg);
}

// qsort.

typedef struct {
  int (*compar)(const void *, const void *);
} qsort_qsort_r_arg_t;

static int _qsort_qsort_r_compar(const void *const x, const void *const y,
                                 qsort_qsort_r_arg_t *const arg) {
  return arg->compar(x, y);
}

void qsort(void *const base, const size_t nmemb, const size_t size,
           int (*const compar)(const void *, const void *)) {
  qsort_qsort_r_arg_t arg = {.compar = compar};
  qsort_r(base, nmemb, size,
          (int (*)(const void *, const void *, void *))_qsort_qsort_r_compar,
          &arg);
}

// qsort_r.

void qsort_r(void *const base, const size_t nmemb, const size_t size,
             int (*const compar)(const void *, const void *, void *),
             void *const arg) {
  _quicksort(base, nmemb, size, compar, arg);
}
