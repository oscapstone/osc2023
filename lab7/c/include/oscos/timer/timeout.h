#ifndef OSCOS_TIMER_TIMEOUT_H
#define OSCOS_TIMER_TIMEOUT_H

#include <stdbool.h>
#include <stdint.h>

void timeout_init(void);

bool timeout_add_timer_ns(void (*callback)(void *), void *arg,
                          uint64_t after_ns);

bool timeout_add_timer_ticks(void (*callback)(void *), void *arg,
                             uint64_t after_ticks);

void xcpt_core_timer_interrupt_handler(void);

#endif
