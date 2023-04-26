#ifndef EXCEPTION_H
#define EXCEPTION_H

// https://github.com/Tekki/raspberrypi-documentation/blob/master/hardware/raspberrypi/bcm2836/QA7_rev3.4.pdf p16
#define CORE0_INTERRUPT_SOURCE ((volatile unsigned int*)(0x40000060))

/*
The basic pending register shows which interrupt are pending. To speed up interrupts processing, a
number of 'normal' interrupt status bits have been added to this register. This makes the 'IRQ
pending base' register different from the other 'base' interrupt registers
p112-115 https://cs140e.sergio.bz/docs/BCM2837-ARM-Peripherals.pdf
*/
#define PBASE 0x3F000000
#define IRQ_BASIC_PENDING	((volatile unsigned int*)(PBASE+0x0000B200))
#define IRQ_PENDING_1		((volatile unsigned int*)(PBASE+0x0000B204))
#define IRQ_PENDING_2		((volatile unsigned int*)(PBASE+0x0000B208))
#define FIQ_CONTROL		    ((volatile unsigned int*)(PBASE+0x0000B20C))
#define ENABLE_IRQS_1		((volatile unsigned int*)(PBASE+0x0000B210))
#define ENABLE_IRQS_2		((volatile unsigned int*)(PBASE+0x0000B214))
#define ENABLE_BASIC_IRQS	((volatile unsigned int*)(PBASE+0x0000B218))
#define DISABLE_IRQS_1		((volatile unsigned int*)(PBASE+0x0000B21C))
#define DISABLE_IRQS_2		((volatile unsigned int*)(PBASE+0x0000B220))
#define DISABLE_BASIC_IRQS	((volatile unsigned int*)(PBASE+0x0000B224))

#define IRQ_PENDING_1_AUX_INT (1<<29)
#define INTERRUPT_SOURCE_GPU (1<<8)
#define INTERRUPT_SOURCE_CNTPNSIRQ (1<<1)
//#define SYSTEM_TIMER_MATCH_1 (1<<1)
//#define SYSTEM_TIMER_MATCH_3 (1<<3)
//#define UART_INT (1ULL<<57)

// store information about the state of a process or thread 
// when an exception or interrupt occurs.
typedef struct trapframe
{
    unsigned long x0;
    unsigned long x1;
    unsigned long x2;
    unsigned long x3;
    unsigned long x4;
    unsigned long x5;
    unsigned long x6;
    unsigned long x7;
    unsigned long x8;
    unsigned long x9;
    unsigned long x10;
    unsigned long x11;
    unsigned long x12;
    unsigned long x13;
    unsigned long x14;
    unsigned long x15;
    unsigned long x16;
    unsigned long x17;
    unsigned long x18;
    unsigned long x19;
    unsigned long x20;
    unsigned long x21;
    unsigned long x22;
    unsigned long x23;
    unsigned long x24;
    unsigned long x25;
    unsigned long x26;
    unsigned long x27;
    unsigned long x28;
    unsigned long x29;
    unsigned long x30;
    unsigned long spsr_el1; // the processor state 
    unsigned long elr_el1;  // the address of the instruction that caused the exception
    unsigned long sp_el0; // the stack pointer for the current process or thread

} trapframe_t;

void sync_64_router(trapframe_t *tpf);
void irq_router(trapframe_t *tpf);
void invalid_exception_router();

static inline void enable_interrupt()
{
    __asm__ __volatile__("msr daifclr, 0xf");
}

static inline void disable_interrupt()
{
    __asm__ __volatile__("msr daifset, 0xf");
}

unsigned long long is_disable_interrupt();
void lock();
void unlock();

#endif