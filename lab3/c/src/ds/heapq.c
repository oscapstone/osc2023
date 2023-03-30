#include "oscos/ds/heapq.h"

#include "oscos/libc/string.h"

// TODO: Check integer overflows.

static void _heap_sift_up(void *const base, const size_t size, const size_t i,
                          int (*const compar)(const void *, const void *,
                                              void *),
                          void *const arg) {
  size_t ic = i;
  while (ic != 0) {
    const size_t ip = (ic - 1) / 2;
    char *const icp = (char *)base + ic * size, *const ipp =
                                                    (char *)base + ip * size;
    if (compar(icp, ipp, arg) < 0) {
      // Swap base[ic] and base[ip].
      for (size_t i = 0; i < size; i++) {
        const char tmp = icp[i];
        icp[i] = ipp[i];
        ipp[i] = tmp;
      }

      ic = ip;
    } else {
      break;
    }
  }
}

static void _heap_sift_down(
    void *const base, const size_t nmemb, const size_t size, const size_t i,
    int (*const compar)(const void *, const void *, void *), void *const arg) {
  size_t ic = i, ic1;
  while ((ic1 = ic * 2 + 1) < nmemb) {
    const size_t ic2 = ic1 + 1;
    char *const icp = (char *)base + ic * size,
                *const ic1p = (char *)base + ic1 * size,
                *const ic2p = (char *)base + ic2 * size;

    size_t ict;
    char *ictp;
    if (compar(ic1p, ic2p, arg) < 0) {
      ict = ic1;
      ictp = ic1p;
    } else {
      ict = ic2;
      ictp = ic2p;
    }

    if (compar(ictp, icp, arg) < 0) {
      // Swap base[ict] and base[ic].
      for (size_t i = 0; i < size; i++) {
        const char tmp = ictp[i];
        ictp[i] = icp[i];
        icp[i] = tmp;
      }

      ic = ict;
    } else {
      break;
    }
  }
}

void heappush(void *const base, const size_t nmemb, const size_t size,
              const void *const item,
              int (*const compar)(const void *, const void *, void *),
              void *const arg) {
  memcpy((char *)base + nmemb * size, item, size);
  _heap_sift_up(base, size, nmemb, compar, arg);
}

void heappop(void *base, size_t nmemb, size_t size, void *item,
             int (*compar)(const void *, const void *, void *), void *arg) {
  if (item) {
    memcpy(item, base, size);
  }
  memcpy(base, (const char *)base + (nmemb - 1) * size, size);
  _heap_sift_down(base, nmemb - 1, size, 0, compar, arg);
}
