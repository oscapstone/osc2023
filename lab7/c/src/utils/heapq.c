#include "oscos/utils/heapq.h"

#include "oscos/libc/string.h"

// TODO: Check integer overflows.

static void _heap_sift_up(void *const base, const size_t size, const size_t i,
                          int (*const compar)(const void *, const void *,
                                              void *),
                          void *const arg) {
  size_t ic = i;
  while (ic != 0) {
    const size_t ip = (ic - 1) / 2;
    void *const pc = (char *)base + ic * size, *const pp =
                                                   (char *)base + ip * size;
    if (compar(pc, pp, arg) < 0) {
      memswp(pc, pp, size);
      ic = ip;
    } else {
      break;
    }
  }
}

static void _heap_sift_down(
    void *const base, const size_t nmemb, const size_t size, const size_t i,
    int (*const compar)(const void *, const void *, void *), void *const arg) {
  size_t ic = i, il;
  while ((il = ic * 2 + 1) < nmemb) {
    const size_t ir = il + 1;
    void *const pc = (char *)base + ic * size,
                *const pl = (char *)base + il * size,
                *const pr = (char *)base + ir * size;

    size_t it;
    void *pt;
    if (ir < nmemb && compar(pl, pr, arg) >= 0) {
      it = ir;
      pt = pr;
    } else {
      it = il;
      pt = pl;
    }

    if (compar(pt, pc, arg) < 0) {
      memswp(pt, pc, size);
      ic = it;
    } else {
      break;
    }
  }
}

void heappush(void *const restrict base, const size_t nmemb, const size_t size,
              const void *const restrict item,
              int (*const compar)(const void *, const void *, void *),
              void *const arg) {
  memcpy((char *)base + nmemb * size, item, size);
  _heap_sift_up(base, size, nmemb, compar, arg);
}

void heappop(void *const restrict base, const size_t nmemb, const size_t size,
             void *const restrict item,
             int (*const compar)(const void *, const void *, void *),
             void *const arg) {
  if (nmemb == 0)
    __builtin_unreachable();

  if (item) {
    memcpy(item, base, size);
  }

  if (nmemb > 1) {
    memcpy(base, (const char *)base + (nmemb - 1) * size, size);
    _heap_sift_down(base, nmemb - 1, size, 0, compar, arg);
  }
}
