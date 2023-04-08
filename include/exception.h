#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#define BASE 0x3F00B000
#define IRQ_PENDING_1 ((volatile unsigned int*)(BASE + 0x0204))



#define SRC_CORE0_INTERRUPT ((volatile unsigned int*)0x40000060)


#define GPU_IRQ_ENTRY (1 << 29)
#define SRC_GPU_INTERRUPT (1 << 8)
#define SRC_CPU_TIMER (1 << 1)
void exception_entry();
void boot_time_2_sec();
void irq_exception_router();
void async_test();

#endif 