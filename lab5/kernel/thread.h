#ifndef THREAD_H
#define THREAD_H

void init_thread(void);
unsigned int thread_create(void (*thread_func)(void));
void schedule(void);

void demo_thread(int num_thread);

#endif /* THREAD_H */