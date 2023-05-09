#include "thread.h"
#include "mem.h"
#include "syscall.h"
#include "terminal.h"
#include "time.h"
#include "uart.h"
#include "vm.h"
#include "str.h"
// Functions in switch.S
extern void switch_to(Thread *, Thread *);
extern Thread *get_current(void);

//=======================================================
// thread queues
extern Thread_q running = {NULL, NULL};
extern Thread_q waiting = {NULL, NULL};
extern Thread_q deleted = {NULL, NULL};
static thread_count = 0;
static Thread *startup;

//========================================================
// Some private functions

void thread_init() {
  startup = pmalloc(1);
  startup->id = thread_count++;
  running.begin = NULL;
  running.end = NULL;
  waiting.begin = NULL;
  waiting.end = NULL;
  deleted.begin = NULL;
  deleted.end = NULL;
  asm volatile("msr tpidr_el1, %[startup];" ::[startup] "r"(startup));
  return;
}
void thread_q_add(Thread_q *Q, Thread *t) {
	disable_int();
  t->prev = NULL;
  t->next = Q->begin;
  if (Q->begin != NULL)
    Q->begin->prev = t;
  Q->begin = t;
  if (Q->end == NULL) {
    Q->end = t;
  }
  enable_int();
  return;
}

Thread *thread_q_pop(Thread_q *Q) {
	disable_int();
  Thread *ret = Q->end;
  if (ret != NULL) {
    Q->end = ret->prev;
    if (Q->begin == ret) {
      Q->begin = ret->prev;
    }
    ret->prev = NULL;
    ret->next = NULL;
  }
  enable_int();
  return ret;
}

// FIXME: Use a better algorithm
Thread *thread_q_delete(Thread_q *Q, Thread *target) {
	disable_int();
  Thread *t = NULL;
  Thread *s = Q->begin;
  t = thread_q_pop(Q);
  while (t != target && t != NULL) {
    thread_q_add(Q, t);
    t = thread_q_pop(Q);
    if (t == s && t != target) {
      thread_q_add(Q, t);
      return NULL;
    }
  }
  enable_int();
  return t;
}

Thread *thread_q_delete_id(Thread_q *Q, int id) {
	disable_int();
  Thread *t = NULL;
  Thread *s = Q->begin;
  t = thread_q_pop(Q);
  while (t->id != id && t != NULL) {
    thread_q_add(Q, t);
    t = thread_q_pop(Q);
    if (t == s && t->id != id) {
      thread_q_add(Q, t);
      // uart_puts("\nDelete by id fail: ");
      return NULL;
    }
  }
  enable_int();
  return t;
}

void vm_base_switch(Thread *next){
	//uart_puti(next->id);
	//uart_puthl(next->pgd);
	asm volatile(
		//"mov	x0,  %[pgd];"
		//"dsb	ish;"
		//"msr	ttbr0_el1, x0;"
		"msr	ttbr0_el1, %[pgd];"
		"msr	ttbr1_el1, %[zero];"
		"tlbi	vmalle1is;"
		"dsb	ish;"
		"isb;" 
		"ret;":: [pgd] "r" (next->pgd), [zero] "r" (0));

	return;
}


/************************************************************************
 * Create thread with a separate kernel SP and user SP.
 *
 * @fn: The first function will be execute after thread created.
 ***********************************************************************/
Thread *thread_create(void (*fn)(void *)) {
  disable_int();
  Thread *cur = (Thread*) pmalloc(0); // Get the small size
  cur = phy2vir(cur);		// This is also in the kernel
  cur->pgd = pmalloc(0);	// Get the entry of PGD
  memset(phy2vir(cur->pgd), 0, 0x1000);
  //uart_puts("\npgd: ");
  //uart_puth(cur->pgd);
  cur->child = 0;
  cur->handler = NULL;
  cur->regs.lr = fn;
  cur->regs.sp = ((char *)cur) + 0x1000 - 16; // The stack will grow lower
  cur->regs.fp = ((char *)cur) + 0x1000 - 16; // No matter?
  cur->id = thread_count++;                   // Set ID
  cur->status = run;                          // Set the status
  cur->signaled = 0;
  cur->vm_list = NULL;
  cur->mapped = 0;
  
  // Note: This sp is in user space, don't add 0xffff0000
  uint64_t tmp_sp = pmalloc(2);
  //uart_puthl(&(cur->vm_list));
  for(int i = 0; i < 4; i++)
	  vm_list_add(phy2vir(&(cur->vm_list)), 0xffffffffb000 + 0x1000 * i, tmp_sp + 0x1000 * i);
  //vm_list_add(cur->vm_list, 0xffffffffb000, tmp_sp, 4); // Demand Paging
  //map_vm(phy2vir(cur->pgd), 0xffffffffb000, tmp_sp, 4); // Map the phyaddr to vm
  //cur->sp_el0 = pmalloc(2) + 0x1000 - 16;     // Separate kernel stack
  cur->sp_el0 = 0xffffffffeff0;	// This is virtual address ,
  cur->sp_el0_kernel = phy2vir(tmp_sp);	// For fork
  thread_q_add(&running, cur);                // Add to thread queue
  enable_int();
  //disable_int();
  return cur;
}

void idle(void) {
  while (1) {
	  //uart_puts("idle");
    kill_zombies();
    schedule();
  }
  return;
}

/*************************************************************************
 * Find if there exist the child is still running but parent is deleted
 *************************************************************************/
void kill_zombies(void) {
  Thread *t = thread_q_pop(&deleted);
  while (t != NULL) {
    // uart_puts("\nkill zombies: ");
    // uart_puti(t->child);
    if (t->child > t->id)
      sys_kill(t->child);
    //pfree(t->sp_el0);
    //pfree(t);
    t = thread_q_pop(&deleted);
  }
  return;
}

/************************************************************************
 * Switch to another thread in running queue
 *
 * If no availiable thread in running queue, just create a terminal therad
 * and run.
 *************************************************************************/
void schedule() {
  Thread *t = thread_q_pop(&running);
  thread_q_add(&running, t);
  // RR
  if (t == NULL) {
    terminal_run_thread();
    idle();
  }
  if (t == running.begin && t == running.end) {
    if (t->status == wait) {
      thread_q_add(&deleted, thread_q_pop(&running));
      terminal_run_thread();
      t = thread_q_pop(&running);
      thread_q_add(&running, t);
      //vm_base_switch(t);
    }
    // sys_kill(t->id);
    // idle();
  }
  Thread *cur = get_current();
  //vm_base_switch(get_current);
  if (cur != NULL) {
	disable_int();
    vm_base_switch(t);
    switch_to(cur, t);
    /*
    uart_puts("switch to: ");
    Thread *tt = get_current();
    uart_puti(tt->id);
    uart_puth(tt->regs.sp);
    uart_puth(tt);
    */
    //uart_puts("switch");
  } else {
	disable_int();
	  /*
    asm volatile(
	"mov	x0, %[cur];"
	"mov	x1, %[nex];"
	"bl 	switch_to;"
	:: [cur] "r" (cur), [nex] "r" (t));
	*/
    //uart_puts("initial switch\n");
    vm_base_switch(t);
    switch_to(startup, t);
    /*
    Thread *tt = get_current();
    uart_puts("switch to : ");
    uart_puti(tt->id);
    uart_puth(tt->regs.sp);
    uart_puth(tt);
    */
    //uart_puts("switch");
  }

  //print_esr_el1();
  enable_int();
  return; // This return may never used
}

/***********************************************************************
 * Move current thread from running queue to deleted queue
 **********************************************************************/
void exit() {
  Thread *t = get_current();
  thread_q_delete(&running, t);
  thread_q_add(&deleted, t);
	uart_puts("exit");
  schedule();
  return;
}

/************************************************************************
 * Test foo()
 ***********************************************************************/
void foo(void *a) {
  Thread *t = get_current();
  for (int i = 0; i < 10; i++) {
    uart_puts("Thread id:");
    uart_puti(t->id);
    uart_puts(", iter: ");
    uart_puti(i);
    uart_puts("\n");
    delay(1000000);
    schedule();
  }
  exit();
  return;
}

/*************************************************************************
 * Test function
 ************************************************************************/
void test_thread_queue(void) {
  asm volatile("msr	tpidr_el1,	%[startup];" ::[startup] "r"(&startup));
  for (int i = 0; i < 4; i++) {
    thread_create(foo);
  }
  idle();
  return;
}

/*************************************************************************
 * Wrapper function of the thread create fo terminal_run()
 ************************************************************************/
void terminal_run_thread() {
  uart_puts("Terminal Thread Create\n");
  Thread *t = thread_create(terminal_run);
  pfree(t->pgd);
  t->pgd = 0x0;	// Using the kernel's pgd 
  return;
}
