#ifndef _EXCEPTION_H
#define _EXCEPTION_H
extern void disable_local_fiq_interrupt(void);
extern void disable_local_irq_interrupt(void);
extern void disable_local_async_interrupt(void);
extern void disable_local_dbg_interrupt(void);
extern void disable_local_all_interrupt(void);
extern void enable_local_fiq_interrupt(void);
extern void enable_local_irq_interrupt(void);
extern void enable_local_async_interrupt(void);
extern void enable_local_dbg_interrupt(void);
extern void enable_local_all_interrupt(void);
extern void set_exception_vector_table(void);
extern void exception_handler(void);
extern void set_exception_vector_table(void);
extern void default_handler(void);
#endif