#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include <stddef.h>

// Store the thread status
enum Thread_status{
	run = 0,
	wait,
	dead
};

// Collect all register's value.
typedef struct{
	uint64_t x19;
	uint64_t x20;
	uint64_t x21;
	uint64_t x22;
	uint64_t x23;
	uint64_t x24;
	uint64_t x25;
	uint64_t x26;
	uint64_t x27;
	uint64_t x28;
	uint64_t fp; // x29, pointer for the frame record to caller.
	uint64_t lr; // x30, return address
	uint64_t sp; // Stack pointer At el1, store the information of the thread structure and the trap frame.
} callee_regs;

typedef struct{
	uint64_t regs[32];	// x0-x30, 16bytes align, exp.S
	uint64_t spsr_el1;
	uint64_t elr_el1;
	uint64_t sp_el0;
	uint64_t Dummy;		// Align for stack
}Trap_frame;

// Data structure of threads in kernel.
typedef struct THread{
	callee_regs regs;	// NOTE: Always first in this struct
	uint32_t id;
	uint32_t child;
	uint64_t sp_el0;	// Store the base_sp at el0
	uint64_t handler;
	enum Thread_status status;
	struct THread *prev;
	struct THread *next;
}Thread;

// Data structure of the thread Queue.
typedef struct THreadQueue{
	Thread *begin;
	Thread *end;
}Thread_q;

/// Register the thread
Thread* thread_create(void (*fp)(void*));
/// Tell scheduler to get running another thread
void idle(void);
/// Cleanup zombies
void kill_zombies(void);
/// Call shchedule to work
void schedule();
/// Move the thread to deaded Q
void exit();
/// Shell thread Init
void terminal_run_thread();

/// Test 
void test_thread_queue();
void thread_init();

/// Aux functions
Thread* thread_q_delete_id(Thread_q*, int );
#endif // THREAD_H
