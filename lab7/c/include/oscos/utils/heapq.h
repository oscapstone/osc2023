#ifndef OSCOS_UTILS_HEAPQ_H
#define OSCOS_UTILS_HEAPQ_H

#include <stddef.h>

void heappush(void *restrict base, size_t nmemb, size_t size,
              const void *restrict item,
              int (*compar)(const void *, const void *, void *), void *arg);

void heappop(void *restrict base, size_t nmemb, size_t size,
             void *restrict item,
             int (*compar)(const void *, const void *, void *), void *arg);

#endif
