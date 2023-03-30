#ifndef OSCOS_DS_HEAPQ_H
#define OSCOS_DS_HEAPQ_H

#include <stddef.h>

void heappush(void *base, size_t nmemb, size_t size, const void *item,
              int (*compar)(const void *, const void *, void *), void *arg);

void heappop(void *base, size_t nmemb, size_t size, void *item,
             int (*compar)(const void *, const void *, void *), void *arg);

#endif
