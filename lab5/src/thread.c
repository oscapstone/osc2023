#include "thread.h"
#include "time.h"
#include "uart.h"
#include "mem.h"
// Functions in switch.S
extern void switch_to(Thread *, Thread*);
extern Thread* get_current(void);

//=======================================================
// thread queues
static Thread_q running = {NULL, NULL};
static Thread_q waiting = {NULL, NULL};
static Thread_q deleted = {NULL, NULL};
static thread_count = 0;
static Thread startup;


//========================================================
// Some private functions

void thread_q_add(Thread_q *Q, Thread* t){
	t->next = Q->begin;
	if(Q->begin != NULL)
		Q->begin->prev = t;
	Q->begin = t;
	if(Q->end == NULL){
		Q->end = t;
	}
	return;
}

Thread* thread_q_pop(Thread_q *Q){
	Thread* ret = Q->end;
	uart_puthl(ret);
	if(ret != NULL){
		Q->end = ret->prev;
		ret->prev = NULL;
	}
	return ret;
}

Thread* thread_create(void (*fn)(void*)){
	Thread *cur = pmalloc(0);	// Get the small size
	cur->regs.lr = fn;
	cur->regs.sp = cur + 0x1000 - 1;	// The stack will grow lower
	cur->regs.fp = cur + 0x1000 - 1;	// FIXME:No matter?
	cur->id = thread_count++;	// Set ID
	cur->status = run;	// Set the status
	thread_q_add(&running, cur); // Add to thread queue
	return cur;
}

void idle(void){
	while(1){
		uart_puts("idle()\n");
		kill_zombies();
		schedule();
	}
	return;
}

void kill_zombies(void){
	//TODO
	return;
}

void schedule(){
	Thread* t = thread_q_pop(&running);
	// RR
	thread_q_add(&running, t);
	if(t == NULL){
		idle();
		schedule();
	}
	Thread* cur = get_current();
	if(cur != NULL){
		switch_to(cur, t);
	}
	return; // This return may never used
}
void foo(void* a){
	Thread *t = get_current();
	for(int i = 0; i < 10; i++){
		uart_puts("Thread id:");
		uart_puti(t->id);
		uart_puts(", iter: ");
		uart_puti(i);
		uart_puts("\n");
		delay(1000000);
		schedule();
	}
	return;
}

void test_thread_queue(void){
	// For Root thread
	startup.id = thread_count ++;
	asm volatile(
		"msr	tpidr_el1,	%[startup];"
		:: [startup] "r" (&startup)
	);
	for(int i = 0; i < 4; i++){
		uart_puthl(foo);
		thread_create(foo);
	}
	idle();
}
