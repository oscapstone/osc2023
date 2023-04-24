#ifndef _EXCEPTION_H
#define _EXCEPTION_H

void enable_interrupt();
void disable_interrupt();
void exception_entry ();
void sync_64_router ();
void irq_exc_router ();
void two_btime_handler ();

#endif