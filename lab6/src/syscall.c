#include "syscall.h"
#include "initrd.h"
#include "loader.h"
#include "mailbox.h"
#include "thread.h"
#include "time.h"
#include "uart.h"
#include "vm.h"
#include "mem.h"
// FIXME: should disable INT in the critical section.

// k From switch.S
extern Thread *get_current();

// From exp.S
/// Load register back and eret
extern void load_reg_ret(void);

// From thread.c
extern Thread_q *running, deleted;

/********************************************************************
 * Systemcall getpid()
 ********************************************************************/
int sys_getpid(void) {
  Thread *ret = get_current();
  // Log
  /*
  uart_puts("getpid: ");
  uart_puti(ret->id);
  */
  return ret->id;
}

/*************************************************************************
 * System call Read()
 ***********************************************************************/
size_t sys_uart_read(char *buf, size_t size) {
  char a;
  char *pivot = buf;
  int count = 0;
  // If get '\n' break
  for (int i = 0; i < size; i++) {
    *pivot++ = uart_getc();
    if (*(pivot - 1) == '\n')
      break;
    count++;
  }
  *pivot = 0;
  /*
  uart_puts("\n[get]: ");
  uart_puts(buf);
  uart_puti(count);
  uart_puti(size);
  */
  return count;
}

/***********************************************************************
 * System call write
 **********************************************************************/
size_t sys_uart_write(const char *buf, size_t size) {
  /*
  uart_puts("\n[write]");
  uart_puth(buf);
  uart_puti(size);
  */
  const char *t = buf;
  for (int i = 0; i < size; i++) {
    uart_putc(*t++);
  }
  return size;
}

/**********************************************************************
 * Execute the target program
 * @name: name of the program in initramfs
 * @argv: Not implement yet
 ********************************************************************/
int sys_exec(const char *name, char *const argv[]) {
  char *start = (char *)initrd_content_getLo(name);
  int size = initrd_content_getSize(name);
  // Get memory for user program.
  char *dest = (char *)pmalloc(6);
  Thread *t = get_current();
  map_vm(t->pgd, 0, dest, 64);	// Map the program to 0x0
  setup_program_loc(dest);
  char *d = dest;
  for (int i = 0; i < size; i++) {
    *d++ = *start++;
  }
  if (size != 0) {
    sys_run_program();
    return 0;
  }
  return 1;
}

/**********************************************************************
 * system call exit()
 * call the `exit()` in therad.h
 ********************************************************************/
void sys_exit(int status) {
  // Currently not implement the status
  exit();
  return 0;
}

/************************************************************************
 * system call mbox
 ***********************************************************************/
int sys_mbox_call(unsigned char ch, unsigned int *mbox) {
  return sys_mailbox_config(ch, mbox);
}

/************************************************************************
 * kill systemcall. Move thread from running Q to delete Q
 *
 * @pid: the ID of target thread
 ***********************************************************************/
void sys_kill(int pid) {
  Thread *t;
  t = thread_q_delete_id(&running, pid);
  if (t == NULL)
    return;
  thread_q_add(&deleted, t);
  return;
}

/************************************************************************
 * Fork
 * @trap_frame: The SP which point to the current storage of current
 * 		regs which need to copy to child.
 ***********************************************************************/
int sys_fork(Trap_frame *trap_frame) {
  Thread *cur = get_current();
  // Child need to load regs from its trap_frame
  Thread *child = thread_create(load_reg_ret);
  // Copy entire struct from parent -> child
  char *c = (char *)child;
  char *f = (char *)cur;
  // Copy kernel stacks...
  for (int i = sizeof(Thread); i < 0x1000; i++) {
    *(c + i) = *(f + i);
  }
  // Copy callee regs
  // NOTE: the memrory layout of the callee_regs is at the begin of thread
  for (int i = 0; i < sizeof(callee_regs); i++) {
    *(c + i) = *(f + i);
  }

  // Setup Trap_frame of child
  // NOTE: sp always > cur
  /*
  uart_puts("copy trap_frames\n");
  uart_puthl(trap_frame);
  uart_puts(".");
  uart_puthl(cur);
  uart_puts(".");
  uart_puthl(child);
  */

  // Setup the half of handler (maybe useless)
  child->regs.lr = load_reg_ret;
  // Setup stack pointer
  cur->regs.sp = trap_frame;

  // Should at the same offset to the cur.
  child->regs.sp = (((void *)trap_frame) - ((void *)cur) + ((void *)child));
  child->regs.fp = (char *)child + 0x1000 - 16;

  // Copy the handler from parent
  child->handler = cur->handler;

  // Setup the return value for child
  Trap_frame *trap_frame_child = child->regs.sp;

  // Set the x0 and Base fp (EL1) (duplicate)
  trap_frame_child->regs[0] = 0;
  trap_frame_child->regs[29] = child->regs.fp;

  // Get the displacement of userspace stack
  trap_frame_child->sp_el0 =
      (char *)trap_frame->sp_el0 - (char *)cur->sp_el0 + (char *)child->sp_el0;

  // Write child's ID in the x0 of parent
  trap_frame->regs[0] = child->id;
  cur->child = child->id;

  // Copy user stack
  c = (char *)trap_frame_child->sp_el0;
  f = (char *)trap_frame->sp_el0;
  while (f != (char *)cur->sp_el0) {
    *c++ = *f++;
  }
  *c = *f;

  // LOG
  /*
  uart_puts("\n[fork] parent: ");
  uart_puti(cur->id);
  uart_puts(" child:");
  uart_puti(child->id);
  uart_puts("\n");
  */
  return;
}

/*************************************************************************
 * Setup the handler for specific signal
 *
 * @signal: the  pointer of custom handler function.
 * @sig: the target signal.
 ************************************************************************/
void sys_signal(int sig, void (*handler)()) {
  // TODO: Should use a struct to store handlers
  // But for this lab, only one handler will be inserted
  Thread *t = get_current();
  t->handler = handler;
  return;
}

/*************************************************************************
 * __handler store the handler pointer
 ************************************************************************/
static void (*__handler)() = NULL;

/************************************************************************
 * The container function of the POSIX signal handler which provide
 * prologue and the epilogue of the handler.
 *
 * NOTE: Need uexit() to leave EL0 and this handler.
 * NOTE: the handle should run in EL0
 ************************************************************************/
void handler_container() {
  if (__handler == NULL) {
    uart_puts("NO handler!\n");
    return;
  }
  __handler();
  __handler = NULL;
  // The EL0 wapper of the `exit()`
  uexit();
}

/**************************************************************************
 * Implementation of the POSIX signal
 *
 * @pid: The id of the Target thread
 * @sig: Which signal want to send
 *************************************************************************/
void posix_kill(int pid, int sig) {
  Thread *t = NULL;
  t = thread_q_delete_id(&running, pid);
  thread_q_add(&running, t);
  if (t == NULL) {
    uart_puti(pid);
    uart_puts("NO target thread\n");
    return;
  }
  // TODO: Should use a special structure instead of using
  // static. This implementation need to disable INT.
  if (t->handler == NULL)
    return;
  // Setup the pointer which handler_container will execuate.
  __handler = t->handler;
  setup_program_loc(handler_container);
  Thread *h = thread_create(sys_run_program);
  return;
}

//============================================================
// Test Functions.
void fork_test() {
  // printf("\nFork Test, pid %d\n", get_pid());
  uart_puts("\nFork Test, Pid ");
  uart_puti(get_pid());
  uart_puts("\n");
  int cnt = 1;
  int ret = 0;
  if ((ret = fork()) == 0) { // child
    long long cur_sp;
    asm volatile("mov %0, sp" : "=r"(cur_sp));
    // printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(),
    // cnt, &cnt, cur_sp);
    uart_puts("first child pid: ");
    uart_puti(get_pid());
    uart_puts(", cnt: ");
    uart_puti(cnt);
    uart_puts(", ptr: ");
    uart_puthl(&cnt);
    uart_puts(", sp: ");
    uart_puthl(cur_sp);
    uart_puts("\n");
    ++cnt;

    if ((ret = fork()) != 0) {
      asm volatile("mov %0, sp" : "=r"(cur_sp));
      // printf("first child pid: %d, cnt: %d, ptr: %x, sp : %x\n", get_pid(),
      // cnt, &cnt, cur_sp);
      uart_puts("first child pid: ");
      uart_puti(get_pid());
      uart_puts(", cnt: ");
      uart_puti(cnt);
      uart_puts(", ptr: ");
      uart_puthl(&cnt);
      uart_puts(", sp: ");
      uart_puthl(cur_sp);
      uart_puts("\n");
    } else {
      while (cnt < 5) {
        asm volatile("mov %0, sp" : "=r"(cur_sp));
        // printf("second child pid: %d, cnt: %d, ptr: %x, sp : %x\n",
        // get_pid(), cnt, &cnt, cur_sp);
        uart_puts("second child pid: ");
        uart_puti(get_pid());
        uart_puts(", cnt: ");
        uart_puti(cnt);
        uart_puts(", ptr: ");
        uart_puthl(&cnt);
        uart_puts(", sp: ");
        uart_puthl(cur_sp);
        uart_puts("\n");
        delay(1000000);
        ++cnt;
      }
    }
    uexit();
  } else {
    // printf("parent here, pid %d, child %d\n", get_pid(), ret);
    uart_puts(" parent here, pid: ");
    uart_puti(get_pid());
    uart_puts(", child: ");
    uart_puti(ret);
    uart_puts("\n");
  }
  uexit();
}

//========================================================================
// Wrapper funtion for user to call. Which contain the svc and x8
//========================================================================

/*************************************************************************
 * Getpid
 * @return: return the pid of the current thread
 ************************************************************************/
int get_pid() {
  uint32_t ret;
  asm volatile("mov x8, 0;"
               "svc 0;"
               "mov %[ret], x0;"
               : [ret] "=r"(ret)
               :);
  return ret;
}

/*************************************************************************
 * Exit, the end of the thread, which should be called at the end of thread
 * **********************************************************************/
void uexit() {

  asm volatile("mov	x8, 5;"
               "svc	5;" ::);
  return;
}

/*************************************************************************
 * Fork, fork the current thread
 ************************************************************************/
int fork() {
  int ret;
  asm volatile("mov	x8, 4;"
               "svc	4;"
               "mov	%[ret] , x0;"
               : [ret] "=r"(ret)
               :);
  return ret;
}
