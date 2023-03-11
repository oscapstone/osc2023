#ifndef MY_STDLIB_H
#define MY_STDLIB_H



typedef unsigned int size_t;

#define BASE ((volatile unsigned int*)(0x60000))
#define LIMIT ((volatile unsigned int*)(0x7FFFF))

void *simple_malloc(size_t size);

#endif