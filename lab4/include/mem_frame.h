#ifndef MEM_FRAME_H
#define MEM_FRAME_H

#define MEM_START       (unsigned long)0x10000000
#define MEM_END         (unsigned long)0x20000000
#define FRAME_SIZE      0x1000
#define NUM_FRAME       ((MEM_END - MEM_START) / FRAME_SIZE)

#define MAX_ORDER               6

#define FSTATE_ALLOCATED        -1
#define FSTATE_BUDDY            -2
#define FSTATE_NOT_ALLOCATABLE  -3

void init_frames(void);
void* allocate_frame(unsigned int order);
void free_frame(void *ptr);

void demo_frame(void);

#endif /* MEM_FRAME_H */