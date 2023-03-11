#ifndef MY_STDLIB_H
#define MY_STDLIB_H



typedef unsigned int size_t;

#define BASE ((volatile unsigned int*)(0x60000))
#define LIMIT ((volatile unsigned int*)(0x7FFFF))

extern volatile unsigned long available;

void *simple_malloc(size_t size);
int return_available();

#endif