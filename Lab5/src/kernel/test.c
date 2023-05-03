#include "stdlib.h"
#include "dynamic_alloc.h"
#include "page_alloc.h"
#include "thread.h"

void test_mem_alloc()
{
    void *a;
    a = my_malloc(200);
    printf("a = %p\n", a);
    free(a);

    a = my_malloc(200);
    printf("a = %p\n", a);
    free(a);

    a = my_malloc(200);
    printf("a = %p\n", a);
    free(a);

    a = my_malloc(200);
    printf("a = %p\n", a);
    free(a);

    a = my_malloc(200);
    printf("a = %p\n", a);
    free(a);

    return;
}

void test_thread()
{
    thread_info *thread_ret;
    thread_ret = thread_create(test_mem_alloc);
    thread_create(test_mem_alloc);
    thread_create(test_mem_alloc);
    thread_create(test_mem_alloc);
    thread_create(test_mem_alloc);
    thread_create(test_mem_alloc);
    thread_create(test_mem_alloc);
    thread_create(test_mem_alloc);

    debug_task_rq();
    return;
}