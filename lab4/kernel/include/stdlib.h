#ifndef STDLIB_H
#define STDLIB_H

void mem_init();
void *malloc(size_t size);
void free_page(unsigned long int ptr);
void free(void* ptr);

#endif /* STDLIB_H */