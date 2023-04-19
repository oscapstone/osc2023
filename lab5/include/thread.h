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
	uint64_t sp;
} callee_regs;

// Data structure of threads in kernel.
typedef struct THread{
	callee_regs regs;	// NOTE: Always first in this struct
	uint32_t id;
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
/// Get the current pid
void get_pid(void);
/// Cleanup zombies
void kill_zombies(void);
/// Call shchedule to work
void schedule();
/// Move the thread to deaded Q
void exit();

/// Test 
void test_thread_queue();
#endif // THREAD_H
