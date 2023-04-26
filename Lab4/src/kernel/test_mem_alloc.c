#include "dynamic_alloc.h"
#include "page_alloc.h"
#include "stdlib.h"

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