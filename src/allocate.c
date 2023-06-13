#include "allocate.h"
#define HEAPSIZE 0x100000
#define MAX_HEAP_SIZE 0x100000
char heap_space[HEAPSIZE];
static int top = 0;

void* simple_malloc(unsigned int size){
  if(top + size > HEAPSIZE) return (char*)0;
  char* ret = &heap_space[top];
  top += size;
  return ret;
}
