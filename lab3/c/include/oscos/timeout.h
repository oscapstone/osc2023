#ifndef OSCOS_TIMEOUT_H
#define OSCOS_TIMEOUT_H

#include <stdbool.h>
#include <stdint.h>

bool add_timer(void (*callback)(void *), void *arg, uint64_t after_ns);

void core_timer_interrupt_enable_el1(void);

void core_timer_interrupt_handler_el1(void);

#endif
