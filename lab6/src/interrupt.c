#include "interrupt.h"
#include "exception.h"
#include "heap.h"
#include "syscall.h"
#include "thread.h"
#include "timer.h"
#include "uart.h"
#include "vm.h"

extern Thread* get_current();

#if 0
task_q *head = 0;

/************************************************************************
 * Add item to the queue
 ***********************************************************************/
int task_queue_add(int (*fn)(void), int priority) {
  disable_int(); // Disable intererupt
  task_q *cur = (task_q *)malloc(sizeof(task_q));
  //uart_puth(cur);
  cur->fn = fn;
  cur->priority = priority;
  cur->next = head;
  head = cur;
  task_queue_preempt(); // Preemption
  enable_int();         // Enable interrupt.
  return 0;
}

/*************************************************************************
 * Preemption the tasks
 *
 * Note: For a lightweight version, this function should called after a
 * new task added in the task queue.
 ************************************************************************/
int task_queue_preempt(void) {
  task_q *cur = head; // The new task.
  task_q *tmp;
  while (cur->next != 0 && cur->priority > cur->next->priority) {
    if (head == cur) {
      head = cur->next;
    }
    tmp = cur->next->next; // SWAP
    cur->next->next = cur;
    cur->next = tmp;
  }
  return 0;
}

/**************************************************************************
 * Run tasks in the task queue.
 *
 * Note: If possible, need to implement free()
 *************************************************************************/
int task_queue_run(void) {
	disable_int();
	task_q* tmp = head;
	task_q* t;
	head = 0;
	enable_int();
  while (tmp != 0) {
    tmp->fn(); // Execute the service function.
    t = tmp;
    t->next = 0;
    tmp = tmp->next;   // Goto next task
  }
  return 0;
}
#else // Array version
task_q Q[100];

/************************************************************************
 * Add item to the queue
 ***********************************************************************/
int task_queue_add(int (*fn)(void), int priority) {
  int tmp = 0;
  task_q swap;
  int i = 0;
  disable_int(); // Disable intererupt
  for (i = 0; i < 100; i++) {
    if (!Q[i].used) {
      break;
    }
  }
  Q[i].fn = fn;
  Q[i].priority = priority;
  Q[i].used = 1;
  tmp = i;
  // Preemption
  for (i = 0; i < 100; i++) {
    if (i == tmp || Q[i].priority <= Q[tmp].priority)
      break;
    // SWAP
    if (Q[i].used && Q[i].priority > Q[tmp].priority) {
      swap.fn = Q[i].fn;
      swap.priority = Q[i].priority;
      Q[i].fn = Q[tmp].fn;
      Q[i].priority = Q[tmp].priority;
      Q[tmp].fn = swap.fn;
      Q[tmp].priority = swap.priority;
    }
  }
  enable_int(); // Enable interrupt.
  return 0;
}

/**************************************************************************
 * Run tasks in the task queue.
 *
 * Note: If possible, need to implement free()
 *************************************************************************/
int task_queue_run(void) {
  for (int i = 0; i < 100; i++) {
    if (!Q[i].used)
      break;
    Q[i].fn();
    Q[i].used = 0;
  }
  return 0;
}

#endif

/*************************************************************************
 * Enable core timer and set the expired time to 2 seconds.
 *************************************************************************/
int core_timer_enable(void) {
  init_timer_Q();
  asm volatile("mov	x0, 	1;"
               "msr	cntp_ctl_el0, x0;"   // Enable timer
               "mrs	x0, 	cntfrq_el0;" // Get count fequency
               "asr	x0,	x0, 5;"
               "msr	cntp_tval_el0, x0;" // Set expire time
               "mov	x0,	2;"
               "ldr	x1, 	=0x40000040;" // Timer interrupt
               "str	w0,	[x1];"        // Unmask the timer interrupt
               "mrs	x0,	cntkctl_el1;"
               "orr	x0, 	x0, 	1;"
               "msr	cntkctl_el1, 	x0;"

  );
  uart_puts(" CORE TIMER INITIAL\n");
  return 0;
}
int core_timer_disable(void) {
  init_timer_Q();
  asm volatile("mov	x0, 	0;"
               "msr	cntp_ctl_el0, x0;" // Enable timer
  );
  uart_puts(" CORE TIMER DISABLE\n");
  return 0;
}

/**************************************************************************
 * Enable mini uart interrupt.
 * Need to enable AUX int (bit 29)
 * Need to connect the GPU IRQ to CORE0's IRQ
 *************************************************************************/
int mini_uart_interrupt_enable(void) {
  *IRQS1 |= (1 << 29); // Encble aux int
  //*AUX_MU_IER = 0x1;	// Enable aux rx interrupt
  //*GPU_INT_ROUT = 0;	// GPU FIQ&IRQ -> CORE0 FIQ&IRQ
  return 0;
}

/**************************************************************************
 * Timer handler which handle the timer interrupt from current EL.
 *************************************************************************/
int core_timer_handler(void) {
  timer_walk(1);
  asm volatile("mrs	x0,	cntfrq_el0;" // Get the clock frequency
               "asr	x0, 	x0, 	5;"
               "msr	cntp_tval_el0,	x0;" // set tval = cval + x0
  );
  schedule();
  return 0;
}

/**************************************************************************
 * Timer handler which handle the timer interrupt from lower EL
 *************************************************************************/
int low_core_timer_handler(void) {
  // uart_puts(" Timer handler!\n");
  uint64_t time, freq;
  asm volatile("mrs	%[time], cntpct_el0;"
               "mrs	%[freq], cntfrq_el0;"
               : [time] "=r"(time), [freq] "=r"(freq));
  // uart_puts("Current second: ");
  // uart_puthl(time / freq);
  // uart_puts("\n");
  //  timer_walk(2);
  freq = freq >> 5;
  asm volatile("msr	cntp_tval_el0,	%[freq];" // set tval = cval + x0
               ::[freq] "r"(freq));
  schedule();
  return 0;
}

/**************************************************************************
 * Function which get the value of CORE0's IRQ.
 *
 * return: int32_t: the source ID of the interrupt.
 *************************************************************************/
static unsigned int get_core0_irq_source(void) { return *CORE0_IRQ_SOURCE; }

/**************************************************************************
 * Clear the IRQ
 *************************************************************************/
static int clear_core0_irq_source(void) {
  *CORE0_IRQ_SOURCE = 0;
  return 0;
}

/**************************************************************************
 * uart handler. Assign the interrupt to the target service functions.
 *************************************************************************/
static int uart_handler(void) {
  // uart_puts("uart_handler\n");
  // uart_puth(*AUX_MU_IIR);
  //  Only care the 2:1 bit in this register.
  switch (*AUX_MU_IIR & 0x6) {
  case 2:
    disable_uart_transmit_int();
    task_queue_add(uart_transmit_handler, 9);
    // NOTE: Don't enable interupt here, let handler decide.
    break;
  case 4:
    // Recieve should response immediately.
    uart_receive_handler();
    break;
  case 0:
  default:
    uart_puts("Error\n");
    break;
  }
  return 0;
}

/**************************************************************************
 * Disable interrupt in current EL
 *************************************************************************/
int disable_int(void) {
  asm volatile("msr	DAIFSet, 0xF;");
  return 0;
}

/**************************************************************************
 * Enable interrupt in current EL
 *************************************************************************/
int enable_int(void) {
  asm volatile("msr	DAIFClr, 0xf;");
  mini_uart_interrupt_enable();
  return 0;
}

/**************************************************************************
 * IRQ handler which from current EL
 *************************************************************************/
int irq_handler(void) {
  disable_int();
  if (*IRQ_PEND_1 & (1 << 29)) // Uart interrupt
    uart_handler();
  else if (*CORE0_IRQ_SOURCE & 0x2) { // Timer interrupt
    disable_timer_int();
    task_queue_add(core_timer_handler, 10); // Put core timer into queue.
    enable_timer_int();
  } else
    task_queue_add(exception_entry, 10);

  enable_int();
  task_queue_run(); // Run the interrupt handler with INT enable.
  // uart_puts("IRQ_HANDLER END\n");
  return 0;
}

/**************************************************************************
 * IRQ handler which handles the interrupts from lower EL
 *************************************************************************/
int low_irq_handler(void) {
  disable_int();
  // uint32_t source = get_core0_irq_source();
  // uart_puts("IRQ_HANDLER\n");
  // uart_puts("IRQ source: ");
  // uart_puth(get_core0_irq_source());
  // uart_puts("\n");
  // print_spsr_el1();
  // print_elr_el1();
  // print_esr_el1();

  // if(*IRQ_PEND_1 & (1<<29) && *AUX_MU_IIR & 0x1)
  if (*IRQ_PEND_1 & (1 << 29))
    uart_handler();
  else if (*CORE0_IRQ_SOURCE & 0x2)
    low_core_timer_handler();
  else
    exception_entry();

  // uart_puts("IRQ_HANDLER END\n");
  enable_int();
  return 0;
}

static int page_fault_handler(){
	uint64_t esr;
	uint64_t far;
	uint64_t f;
	Thread* t = get_current();
	uint64_t phy;
	t = phy2vir(t);
	asm volatile("mrs %[esr], esr_el1;": [esr] "=r" (esr));
	asm volatile("mrs %[far], far_el1;": [far] "=r" (far));
	f = far & 0x0000fffffffffffff000;
	// ISS is at [5:0]
	esr = esr & 0x2f;
	if(esr >= 4 || esr <= 7){
		uart_puts("[Translation fault] ");
		phy = vm_list_delete(phy2vir(&(t->vm_list)), f);
		if(phy == 0 || far & 0xffff000000000000){
			uart_puts("VM LIST ERROR!!\n");
			uart_puthl(f);
			uart_puthl(phy);
		}
		if(phy != 0)
		map_vm(phy2vir(t->pgd), f, phy, 1);
		uart_puthl(far);
		uart_puts("\n");
	}
	else{
		uart_puts("[Segmantation fault\n");
	}
	return 0;
}
	

/**********************************************************************
 * Low_synchronize_handler (SVC)
 * @trap_fram: The address of sp which store the x0-x30 and
 * 		spsr_el1, elr_el1, sp_el0
 * NOTE: the rerturn value of each functions are written in the
 * 	trap_frame, which will be copy back when exit interrupt routine.
 * ********************************************************************/
void low_syn_handler(Trap_frame *trap_frame) {
  uint64_t esr; // Which contain the exception and value
  uint64_t index;
  uint64_t *regs = trap_frame->regs;
  Thread *t = get_current();
  t = phy2vir(t);
  asm volatile("mrs  %[esr], esr_el1;" : [esr] "=r"(esr) :);
  enable_int();
  // Check if the exception if from SVC
  // Need to check 31-26 bit at esr
  if (((esr >> 26) & 0x7F) == 0x15) {
    index = esr & 0x1FFFFFF; // 24:0 store the index
    switch (regs[8]) {
    case 0:
      regs[0] = sys_getpid();
      break;
    case 1:
      regs[0] = sys_uart_read(regs[0], regs[1]);
      break;
    case 2:
      regs[0] = sys_uart_write(regs[0], regs[1]);
      break;
    case 3:
      regs[0] = sys_exec(regs[0], regs[1]);
      break;
    case 4:
      // Need to pass the trap_frame for copying to child
      // regs[0] = sys_fork(trap_frame);
      disable_int();
      sys_fork(trap_frame);
      enable_int();
      break;
    case 5:
      sys_exit(regs[0]);
      break;
    case 6:
      regs[0] = sys_mbox_call(regs[0], regs[1]);
      break;
    case 7:
      sys_kill(regs[0]);
      break;
    case 8:
      sys_signal(regs[0], regs[1]);
      break;
    case 9:
      posix_kill(regs[0], regs[1]);
      break;
    case 10:
      /*
      uart_puts("mmap: ");
      uart_puthl(regs[0]);
      uart_puts(" ");
      uart_puthl(regs[1]);
      uart_puts(" ");
      uart_puthl(regs[2]);
      uart_puts(" ");
      uart_puthl(regs[3]);
      uart_puts(" ");
      uart_puthl(regs[4]);
      uart_puts(" ");
      uart_puthl(regs[5]);
      uart_puts(" ");
      */
      regs[0] = sys_mmap(regs[0], regs[1], regs[2], regs[3], regs[4], regs[5]);
      break;
    case 15:
      asm volatile(
	"mov	sp, x19;");
      break;
    default:
      uart_puts("***UNKNOWN INT***\n");
    }
  }
  else if (((esr >> 26) & 0x7F) == 0x20) {
	  uart_puts("Instruction abort\n");
	  page_fault_handler();
	  return;
  }
  else if (((esr >> 26) & 0x7F) == 0x24) {
	  uart_puts("Data abort\n");
	  page_fault_handler();
  }
  if(t->signaled){
	  t->signaled = 0;
	  asm volatile("mov	x19, sp;"
			"msr	elr_el1, %[loc];"
			:: [loc] "r" (handler_container));
  }
  return;
}

