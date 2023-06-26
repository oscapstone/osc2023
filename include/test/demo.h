#ifndef __DEMO_H
#define __DEMO_H
#include "type.h"
#include "test/demo_page.h"
#include "test/test_simple_alloc.h"
#include "test/test_kmalloc.h"
#include "test/test_random.h"

void demo_init();
void demo_timer_interrupt();
void demo_preempt(void *ptr, uint32_t sz);
void demo_preempt_vic(void *ptr, uint32_t sz);
void demo_page_alloc();
#endif