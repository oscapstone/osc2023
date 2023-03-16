#ifndef MALLOC_H
#define MALLOC_H

extern char __heap_start;

void *simple_malloc(unsigned long size);

#endif