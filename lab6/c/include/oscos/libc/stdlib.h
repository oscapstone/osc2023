#ifndef OSCOS_LIBC_STDLIB_H
#define OSCOS_LIBC_STDLIB_H

#include <stddef.h>

void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *));
void qsort_r(void *base, size_t nmemb, size_t size,
             int (*compar)(const void *, const void *, void *), void *arg);

#endif
