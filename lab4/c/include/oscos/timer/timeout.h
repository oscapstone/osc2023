#ifndef OSCOS_TIMER_TIMEOUT_H
#define OSCOS_TIMER_TIMEOUT_H

#include <stdbool.h>
#include <stdint.h>

void timeout_init(void);

bool timeout_add_timer(void (*callback)(void *), void *arg, uint64_t after_ns);

void xcpt_core_timer_interrupt_handler(void);

#endif
