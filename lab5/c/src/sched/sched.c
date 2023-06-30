// This module implements a round-robin scheduler.

#include "oscos/sched.h"

#include "oscos/libc/string.h"
#include "oscos/mem/malloc.h"
#include "oscos/mem/page-alloc.h"
#include "oscos/mem/vm.h"
#include "oscos/utils/critical-section.h"
#include "oscos/utils/math.h"
#include "oscos/utils/rb.h"

#define THREAD_STACK_ORDER 13 // 8KB.
#define THREAD_STACK_BLOCK_ORDER (THREAD_STACK_ORDER - PAGE_ORDER)

#define USER_STACK_ORDER 23 // 8MB.
#define USER_STACK_BLOCK_ORDER (USER_STACK_ORDER - PAGE_ORDER)

void _suspend_to_wait_queue(thread_list_node_t *wait_queue);
void _sched_run_thread(thread_t *thread);
void thread_main(void);
noreturn void user_program_main(const void *init_pc, const void *init_user_sp,
                                const void *init_kernel_sp);
noreturn void fork_child_ret(void);
void run_signal_handler(sighandler_t handler, void *init_user_sp);

static size_t _sched_next_tid = 1, _sched_next_pid = 1;
static thread_list_node_t _run_queue = {.prev = &_run_queue,
                                        .next = &_run_queue},
                          _zombies = {.prev = &_zombies, .next = &_zombies},
                          _stopped_threads = {.prev = &_stopped_threads,
                                              .next = &_stopped_threads};
static rb_node_t *_processes = NULL;

static int _cmp_processes_by_pid(const process_t *const *const p1,
                                 const process_t *const *const p2, void *_arg) {
  (void)_arg;

  const size_t pid1 = (*p1)->id, pid2 = (*p2)->id;
  return pid1 < pid2 ? -1 : pid1 > pid2 ? 1 : 0;
}

static int _cmp_pid_and_processes_by_pid(const size_t *const pid,
                                         const process_t *const *const process,
                                         void *_arg) {
  (void)_arg;

  const size_t pid1 = *pid, pid2 = (*process)->id;
  return pid1 < pid2 ? -1 : pid1 > pid2 ? 1 : 0;
}

static size_t _calc_text_block_order(const size_t text_len) {
  const size_t text_n_pages =
      (text_len + ((1 << PAGE_ORDER) - 1)) >> PAGE_ORDER;
  const size_t actual_text_block_order = clog2(text_n_pages);

  // We don't know exactly how large the .bss section of a user program is, so
  // we have to use a heuristic. We speculate that the .bss section is at most
  // as large as the remaining parts (.text, .rodata, and .data sections) of the
  // user program.
  const size_t allocating_text_block_order = actual_text_block_order + 1;

  return allocating_text_block_order;
}

static size_t _alloc_tid(void) {
  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  const size_t result = _sched_next_tid++;

  CRITICAL_SECTION_LEAVE(daif_val);
  return result;
}

static size_t _alloc_pid(void) {
  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  const size_t result = _sched_next_pid++;

  CRITICAL_SECTION_LEAVE(daif_val);
  return result;
}

static void _add_thread_to_queue(thread_t *const thread,
                                 thread_list_node_t *const queue) {
  thread_list_node_t *const thread_node = &thread->list_node;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  thread_list_node_t *const last_node = queue->prev;
  thread_node->prev = last_node;
  last_node->next = thread_node;
  thread_node->next = queue;
  queue->prev = thread_node;

  CRITICAL_SECTION_LEAVE(daif_val);
}

static void _add_thread_to_run_queue(thread_t *const thread) {
  _add_thread_to_queue(thread, &_run_queue);
}

void _remove_thread_from_queue(thread_t *const thread) {
  thread_list_node_t *const thread_node = &thread->list_node;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  thread_node->prev->next = thread_node->next;
  thread_node->next->prev = thread_node->prev;

  CRITICAL_SECTION_LEAVE(daif_val);
}

thread_t *_remove_first_thread_from_queue(thread_list_node_t *const queue) {
  thread_t *result;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  if (queue->next == queue) {
    result = NULL;
  } else {
    thread_list_node_t *const first_node = queue->next;
    queue->next = first_node->next;
    first_node->next->prev = queue;
    result = (thread_t *)((char *)first_node - offsetof(thread_t, list_node));
  }

  CRITICAL_SECTION_LEAVE(daif_val);
  return result;
}

bool sched_init(void) {
  // Create the idle thread.

  thread_t *const idle_thread = malloc(sizeof(thread_t));
  if (!idle_thread)
    return false;

  idle_thread->id = 0;
  idle_thread->process = NULL;
  idle_thread->ctx.fp_simd_ctx = NULL;

  // Name the current thread the idle thread.

  __asm__ __volatile__("msr tpidr_el1, %0" : : "r"(idle_thread));

  return true;
}

bool thread_create(void (*const task)(void *), void *const arg) {
  // Allocate memory.

  thread_t *const thread = malloc(sizeof(thread_t));
  if (!thread)
    return false;

  const spage_id_t stack_page_id = alloc_pages(THREAD_STACK_BLOCK_ORDER);
  if (stack_page_id < 0) {
    free(thread);
    return false;
  }

  // Initialize the thread structure.

  thread->id = _alloc_tid();
  thread->status.is_waiting = false;
  thread->status.is_stopped = false;
  thread->status.is_waken_up_by_signal = false;
  thread->status.is_handling_signal = false;
  thread->process = NULL;
  thread->ctx.r19 = (uint64_t)(uintptr_t)task;
  thread->ctx.r20 = (uint64_t)(uintptr_t)arg;
  thread->ctx.pc = (uint64_t)(uintptr_t)thread_main;
  thread->ctx.fp_simd_ctx = NULL;

  thread->stack_page_id = stack_page_id;
  void *const init_sp = (char *)pa_to_kernel_va(page_id_to_pa(stack_page_id)) +
                        (1 << (THREAD_STACK_BLOCK_ORDER + PAGE_ORDER));
  thread->ctx.kernel_sp = (uint64_t)(uintptr_t)init_sp;

  // Put the thread into the end of the run queue.

  _add_thread_to_run_queue(thread);

  return true;
}

thread_t *
_sched_move_thread_to_queue_and_pick_thread(thread_t *const thread,
                                            thread_list_node_t *const queue) {
  _add_thread_to_queue(thread, queue);
  return _remove_first_thread_from_queue(&_run_queue);
}

void thread_exit(void) {
  thread_t *const curr_thread = current_thread();
  process_t *const curr_process = curr_thread->process;

  XCPT_MASK_ALL();

  if (curr_process) {
    rb_delete(&_processes, &curr_process->id,
              (int (*)(const void *, const void *,
                       void *))_cmp_pid_and_processes_by_pid,
              NULL);
  }

  _sched_run_thread(
      _sched_move_thread_to_queue_and_pick_thread(curr_thread, &_zombies));
  __builtin_unreachable();
}

thread_t *current_thread(void) {
  thread_t *result;
  __asm__ __volatile__("mrs %0, tpidr_el1" : "=r"(result));
  return result;
}

bool process_create(void) {
  // Allocate memory.

  process_t *const process = malloc(sizeof(process_t));
  if (!process) // Out of memory.
    return false;

  const spage_id_t user_stack_page_id = alloc_pages(USER_STACK_BLOCK_ORDER);
  if (user_stack_page_id < 0) { // Out of memory.
    free(process);
    return false;
  }

#ifdef SCHED_ENABLE_SHARED_USER_STACK
  shared_page_t *const user_stack_page = shared_page_init(user_stack_page_id);
  if (!user_stack_page) {
    free_pages(user_stack_page_id);
    free(process);
    return false;
  }
#endif

  thread_fp_simd_ctx_t *const fp_simd_ctx =
      malloc(sizeof(thread_fp_simd_ctx_t));
  if (!fp_simd_ctx) {
#ifdef SCHED_ENABLE_SHARED_USER_STACK
    shared_page_drop(user_stack_page);
#else
    free_pages(user_stack_page_id);
#endif
    free(process);
    return false;
  }

  // Set thread/process data.

  thread_t *const curr_thread = current_thread();

  process->id = _alloc_pid();
#ifdef SCHED_ENABLE_SHARED_USER_STACK
  process->user_stack_page = user_stack_page;
#else
  process->user_stack_page_id = user_stack_page_id;
#endif
  process->signal_stack_pages = NULL;
  process->main_thread = curr_thread;
  process->pending_signals = 0;
  process->blocked_signals = 0;
  for (size_t i = 0; i < 32; i++) {
    process->signal_handlers[i] = SIG_DFL;
  }
  curr_thread->process = process;
  curr_thread->ctx.fp_simd_ctx = fp_simd_ctx;

  // Zero the stack pages.

  void *user_stack_start = pa_to_kernel_va(page_id_to_pa(user_stack_page_id));
  memset(user_stack_start, 0, 1 << USER_STACK_ORDER);

  // Add the process to the process BST.

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  rb_insert(&_processes, sizeof(process_t *), &process,
            (int (*)(const void *, const void *, void *))_cmp_processes_by_pid,
            NULL);

  CRITICAL_SECTION_LEAVE(daif_val);

  return true;
}

static void _exec_generic(const void *const text_start, const size_t text_len,
                          const bool free_text_page) {
  thread_t *const curr_thread = current_thread();
  process_t *const curr_process = curr_thread->process;

  // Allocate memory.

  const size_t text_block_order = _calc_text_block_order(text_len);
  const spage_id_t text_page_id = alloc_pages(text_block_order);
  if (text_page_id < 0) {
    return;
  }

  shared_page_t *const text_page = shared_page_init(text_page_id);
  if (!text_page) {
    free_pages(text_page_id);
    return;
  }

  const spage_id_t user_stack_page_id = alloc_pages(USER_STACK_BLOCK_ORDER);
  if (user_stack_page_id < 0) {
    shared_page_drop(text_page);
    return;
  }

#ifdef SCHED_ENABLE_SHARED_USER_STACK
  shared_page_t *const user_stack_page = shared_page_init(user_stack_page_id);
  if (!user_stack_page) {
    free_pages(user_stack_page_id);
    shared_page_drop(text_page);
    return;
  }
#endif

  // Free old pages.

  if (free_text_page) {
    shared_page_drop(curr_process->text_page);
  }

#ifdef SCHED_ENABLE_SHARED_USER_STACK
  shared_page_drop(curr_process->user_stack_page);
#else
  free_pages(curr_process->user_stack_page_id);
#endif

  // Set process data.

  curr_process->text_page = text_page;
#ifdef SCHED_ENABLE_SHARED_USER_STACK
  curr_process->user_stack_page = user_stack_page;
#else
  curr_process->user_stack_page_id = user_stack_page_id;
#endif

  // Copy the user program.

  void *const user_program_start = pa_to_kernel_va(page_id_to_pa(text_page_id));
  memcpy(user_program_start, text_start, text_len);

  // Zero the remaining spaces of the text pages.
  memset((char *)user_program_start + text_len, 0,
         (1 << (text_block_order + PAGE_ORDER)) - text_len);

  // Zero the stack pages.

  void *user_stack_start = pa_to_kernel_va(page_id_to_pa(user_stack_page_id));
  memset(user_stack_start, 0, 1 << USER_STACK_ORDER);

  // Run the user program.

  void *const user_stack_end =
                  (char *)user_stack_start + (1 << USER_STACK_ORDER),
              *const kernel_stack_end = (char *)pa_to_kernel_va(page_id_to_pa(
                                            curr_thread->stack_page_id)) +
                                        (1 << THREAD_STACK_ORDER);
  user_program_main(user_program_start, user_stack_end, kernel_stack_end);
}

void exec_first(const void *const text_start, const size_t text_len) {
  _exec_generic(text_start, text_len, false);
}

void exec(const void *const text_start, const size_t text_len) {
  _exec_generic(text_start, text_len, true);
}

static void _thread_cleanup(thread_t *const thread) {
  if (thread->process) {
    shared_page_drop(thread->process->text_page);
#ifdef SCHED_ENABLE_SHARED_USER_STACK
    shared_page_drop(thread->process->user_stack_page);
#else
    free_pages(thread->process->user_stack_page_id);
#endif

    page_id_list_node_t *curr_node = thread->process->signal_stack_pages;
    while (curr_node) {
      page_id_list_node_t *const next_node = curr_node->next;
      free(curr_node);
      curr_node = next_node;
    }

    free(thread->process);
  }
  free_pages(thread->stack_page_id);
  free(thread);
}

process_t *fork(const extended_trap_frame_t *const trap_frame) {
  thread_t *const curr_thread = current_thread();
  process_t *const curr_process = curr_thread->process;

  // Allocate memory.

  thread_t *const new_thread = malloc(sizeof(thread_t));
  if (!new_thread)
    return NULL;

  process_t *const new_process = malloc(sizeof(process_t));
  if (!new_process) {
    free(new_thread);
    return NULL;
  }

  const spage_id_t kernel_stack_page_id = alloc_pages(THREAD_STACK_BLOCK_ORDER);
  if (kernel_stack_page_id < 0) {
    free(new_process);
    free(new_thread);
    return NULL;
  }

  thread_fp_simd_ctx_t *const fp_simd_ctx =
      malloc(sizeof(thread_fp_simd_ctx_t));
  if (!fp_simd_ctx) {
    free_pages(kernel_stack_page_id);
    free(new_process);
    free(new_thread);
    return NULL;
  }

#ifndef SCHED_ENABLE_SHARED_USER_STACK
  const spage_id_t user_stack_page_id = alloc_pages(USER_STACK_BLOCK_ORDER);
  if (user_stack_page_id < 0) {
    free(fp_simd_ctx);
    free_pages(kernel_stack_page_id);
    free(new_process);
    free(new_thread);
    return NULL;
  }
#endif

  // Set data.

  new_thread->id = _alloc_tid();
  new_thread->status.is_waiting = false;
  new_thread->status.is_stopped = false;
  new_thread->status.is_waken_up_by_signal = false;
  new_thread->status.is_handling_signal = false;
  new_thread->stack_page_id = kernel_stack_page_id;
  new_thread->process = new_process;

  new_process->id = _alloc_pid();
  new_process->text_page = shared_page_clone(curr_process->text_page);
#ifdef SCHED_ENABLE_SHARED_USER_STACK
  new_process->user_stack_page =
      shared_page_clone(curr_process->user_stack_page);
#else
  new_process->user_stack_page_id = user_stack_page_id;
#endif
  new_process->signal_stack_pages = NULL;
  new_process->main_thread = new_thread;
  new_process->pending_signals = 0;
  new_process->blocked_signals = 0;
  memcpy(new_process->signal_handlers, curr_process->signal_handlers,
         32 * sizeof(sighandler_t));

  // Set execution context.

  void *const kernel_stack_end =
                  (char *)pa_to_kernel_va(page_id_to_pa(kernel_stack_page_id)) +
                  (1 << THREAD_STACK_ORDER),
              *const init_kernel_sp =
                  (char *)kernel_stack_end - sizeof(extended_trap_frame_t);

  memcpy(&new_thread->ctx, &curr_thread->ctx, sizeof(thread_ctx_t));
  new_thread->ctx.pc = (uint64_t)(uintptr_t)fork_child_ret;
  new_thread->ctx.kernel_sp = (uint64_t)(uintptr_t)init_kernel_sp;
  new_thread->ctx.fp_simd_ctx = fp_simd_ctx;
  memcpy(fp_simd_ctx, curr_thread->ctx.fp_simd_ctx,
         sizeof(thread_fp_simd_ctx_t));

  memcpy(init_kernel_sp, trap_frame, sizeof(extended_trap_frame_t));

#ifndef SCHED_ENABLE_SHARED_USER_STACK
  // Setup the user stack.

  void *const old_user_stack_start = pa_to_kernel_va(
                  page_id_to_pa(curr_process->user_stack_page_id)),
              *const user_stack_start =
                  pa_to_kernel_va(page_id_to_pa(user_stack_page_id)),
              *const new_sp =
                  (char *)user_stack_start + ((char *)curr_thread->ctx.user_sp -
                                              (char *)old_user_stack_start);

  memcpy(user_stack_start, old_user_stack_start, 1 << USER_STACK_ORDER);
  new_thread->ctx.user_sp = (uint64_t)new_sp;
#endif

  // Add the new thread to the end of the run queue and the process to the
  // process BST. Note that these two steps can be done in either order but must
  // not be interrupted in between, since:
  // - If the former is done before the latter and the newly-created thread is
  //   scheduled between the two steps, then the new thread won't be able to
  //   find its own process.
  // - If the latter is done before the former and the newly-created process is
  //   killed between the two steps, then a freed thread_t instance (i.e.,
  //   `new_thread`) will be added onto the run queue – a use-after-free bug.

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  _add_thread_to_run_queue(new_thread);

  rb_insert(&_processes, sizeof(process_t *), &new_process,
            (int (*)(const void *, const void *, void *))_cmp_processes_by_pid,
            NULL);

  CRITICAL_SECTION_LEAVE(daif_val);

  return new_process;
}

void kill_zombies(void) {
  thread_t *zombie;
  while ((zombie = _remove_first_thread_from_queue(&_zombies))) {
    _thread_cleanup(zombie);
  }
}

void schedule(void) { _suspend_to_wait_queue(&_run_queue); }

void suspend_to_wait_queue(thread_list_node_t *const wait_queue) {
  XCPT_MASK_ALL();
  current_thread()->status.is_waiting = true;
  _suspend_to_wait_queue(wait_queue);
}

void wake_up_all_threads_in_wait_queue(thread_list_node_t *const wait_queue) {
  thread_t *thread;
  while ((thread = _remove_first_thread_from_queue(wait_queue))) {
    thread->status.is_waiting = false;
    _add_thread_to_run_queue(thread);
  }
}

process_t *get_process_by_id(const size_t pid) {
  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  process_t *const *const result =
      (process_t **)rb_search(_processes, &pid,
                              (int (*)(const void *, const void *,
                                       void *))_cmp_pid_and_processes_by_pid,
                              NULL);

  CRITICAL_SECTION_LEAVE(daif_val);

  return result ? *result : NULL;
}

void kill_process(process_t *const process) {
  if (process == current_thread()->process) {
    thread_exit();
  } else {
    thread_t *const thread = process->main_thread;

    // Remove the process from the process BST and the thread from the run/wait
    // queue. Note that these two steps can be done in either order but must not
    // be interrupted in between, since:
    // - If the former is done before the latter and the thread is scheduled
    //   between the two steps, then the new thread won't be able to find its
    //   own process.
    // - If the latter is done before the former and the newly-created process
    //   is killed once again between the two steps, then a freed thread_t
    //   instance (i.e., `thread`) will be added onto the zombies list – a
    //   use-after-free bug.

    uint64_t daif_val;
    CRITICAL_SECTION_ENTER(daif_val);

    rb_delete(&_processes, &process->id,
              (int (*)(const void *, const void *,
                       void *))_cmp_pid_and_processes_by_pid,
              NULL);

    _remove_thread_from_queue(thread);
    _add_thread_to_queue(thread, &_zombies);

    CRITICAL_SECTION_LEAVE(daif_val);
  }
}

void kill_all_processes(void) {
  // Kill all processes other than the current one.
  for (;;) {
    uint64_t daif_val;
    CRITICAL_SECTION_ENTER(daif_val);

    process_t *process_to_kill = *(process_t **)_processes->payload;
    if (process_to_kill == current_thread()->process) {
      if (_processes->children[0]) {
        process_to_kill = *(process_t **)_processes->children[0]->payload;
      } else if (_processes->children[1]) {
        process_to_kill = *(process_t **)_processes->children[1]->payload;
      } else { // Only the current process left.
        CRITICAL_SECTION_LEAVE(daif_val);
        break;
      }
    }

    CRITICAL_SECTION_LEAVE(daif_val);

    kill_process(process_to_kill);
  }

  // Kill the current process.
  thread_exit();
}

sighandler_t set_signal_handler(process_t *const process, const int signal,
                                const sighandler_t handler) {
  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  const sighandler_t old_handler = process->signal_handlers[signal];

  // The video player expects SIGKILL to be catchable. This function thus
  // follows the protocol used by the video player.
  if (!(/* signal == SIGKILL || */ signal == SIGSTOP)) {
    process->signal_handlers[signal] = handler;
  }

  CRITICAL_SECTION_LEAVE(daif_val);
  return old_handler;
}

void deliver_signal(process_t *const process, const int signal) {
  thread_t *const thread = process->main_thread;

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  process->pending_signals |= 1 << signal;

  if (thread->status.is_waiting &&
      (!thread->status.is_stopped || signal == SIGCONT)) {
    // Wake up the thread and notify it that it was waken up by a signal.

    thread->status.is_waiting = false;
    thread->status.is_stopped = false;
    thread->status.is_waken_up_by_signal = true;

    _remove_thread_from_queue(thread);
    _add_thread_to_run_queue(thread);
  }

  CRITICAL_SECTION_LEAVE(daif_val);
}

static void _deliver_signal_to_all_processes_rec(const int signal,
                                                 const rb_node_t *const node) {
  if (!node)
    return;

  process_t *const process = *(process_t **)node->payload;
  deliver_signal(process, signal);

  _deliver_signal_to_all_processes_rec(signal, node->children[0]);
  _deliver_signal_to_all_processes_rec(signal, node->children[1]);
}

void deliver_signal_to_all_processes(const int signal) {
  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  _deliver_signal_to_all_processes_rec(signal, _processes);

  CRITICAL_SECTION_LEAVE(daif_val);
}

static bool _signal_default_is_term_or_core(const int signal) {
  return signal == SIGABRT || signal == SIGALRM || signal == SIGBUS ||
         signal == SIGFPE || signal == SIGHUP || signal == SIGILL ||
         signal == SIGINT || signal == SIGIO || signal == SIGIOT ||
         signal == SIGKILL || signal == SIGPIPE || signal == SIGPROF ||
         signal == SIGPWR || signal == SIGQUIT || signal == SIGSEGV ||
         signal == SIGSTKFLT || signal == SIGSYS || signal == SIGTERM ||
         signal == SIGTRAP || signal == SIGUNUSED || signal == SIGUSR1 ||
         signal == SIGUSR2 || signal == SIGVTALRM || signal == SIGXCPU ||
         signal == SIGXFSZ;
}

static bool _signal_default_is_stop(const int signal) {
  return signal == SIGSTOP || signal == SIGTSTP || signal == SIGTTIN ||
         signal == SIGTTOU;
}

void handle_signals(void) {
  thread_t *const curr_thread = current_thread();
  if (!curr_thread) // The scheduler has not been initialized.
    return;
  process_t *const curr_process = curr_thread->process;

  // Save spsr_el1 and elr_el1, since they can be clobbered when running the
  // signal handlers.

  uint64_t spsr_val, elr_val;
  __asm__ __volatile__("mrs %0, spsr_el1" : "=r"(spsr_val));
  __asm__ __volatile__("mrs %0, elr_el1" : "=r"(elr_val));

  // Save the old user SP, since the signal handlers will run in a new user
  // stack.

  uint64_t old_user_sp;
  __asm__ __volatile__("mrs %0, sp_el0" : "=r"(old_user_sp));

  // Save the status bit in order to support nested signal handling.

  const bool is_handling_signal = curr_thread->status.is_handling_signal;

  // The user stack is allocated lazily.

  bool user_stack_allocation_attempted = false;
  spage_id_t user_stack_page_id;
  page_id_list_node_t *const curr_signal_stack_pages_node =
                                 curr_process->signal_stack_pages,
                             *new_signal_stack_pages_node;

  uint32_t deferred_signals = 0;
  for (uint32_t handleable_signals;
       (handleable_signals = curr_process->pending_signals &
                             ~curr_process->blocked_signals &
                             ~deferred_signals) != 0;) {
    uint32_t reversed_handleable_signals;
    __asm__("rbit %w0, %w1"
            : "=r"(reversed_handleable_signals)
            : "r"(handleable_signals));
    int signal;
    __asm__("clz %w0, %w1" : "=r"(signal) : "r"(reversed_handleable_signals));

    const sighandler_t handler = curr_process->signal_handlers[signal];
    if (handler == SIG_DFL) {
      curr_process->pending_signals &= ~(1 << signal);

      if (_signal_default_is_term_or_core(signal)) {
        thread_exit();
      } else if (_signal_default_is_stop(signal)) {
        curr_thread->status.is_stopped = true;
        suspend_to_wait_queue(&_stopped_threads);
        curr_thread->status.is_waken_up_by_signal = false;
      }
    } else if (handler == SIG_IGN) {
      curr_process->pending_signals &= ~(1 << signal);
    } else {
      // Allocate user stack.

      if (!user_stack_allocation_attempted) {
        user_stack_allocation_attempted = true;
        user_stack_page_id = alloc_pages(USER_STACK_BLOCK_ORDER);

        if (user_stack_page_id >= 0) {
          new_signal_stack_pages_node = malloc(sizeof(page_id_list_node_t));
          if (new_signal_stack_pages_node) {
            new_signal_stack_pages_node->page_id = user_stack_page_id;
            new_signal_stack_pages_node->next = curr_signal_stack_pages_node;
            curr_process->signal_stack_pages = new_signal_stack_pages_node;
          } else {
            free_pages(user_stack_page_id);
            user_stack_page_id = -1;
          }
        }
      }

      if (user_stack_page_id >= 0) {
        void *const user_stack_start =
                        pa_to_kernel_va(page_id_to_pa(user_stack_page_id)),
                    *const user_stack_end =
                        (char *)user_stack_start + (1 << USER_STACK_ORDER);

        // Zero the user stack.

        memset(user_stack_start, 0, 1 << USER_STACK_ORDER);

        // Run the signal handler.

        curr_thread->status.is_handling_signal = true;
        curr_process->blocked_signals |= 1 << signal;

        run_signal_handler(handler, user_stack_end);

        curr_thread->status.is_handling_signal = false;
        curr_process->pending_signals &= ~(1 << signal);
        curr_process->blocked_signals &= ~(1 << signal);
      } else {
        // Defer the handling of this signal. Hopefully, there will be enough
        // memory to run the signal handler in the future.
        deferred_signals |= 1 << signal;
      }
    }
  }

  // Destroy the user stack.

  if (user_stack_allocation_attempted && user_stack_page_id >= 0) {
    free_pages(user_stack_page_id);
    curr_process->signal_stack_pages = curr_signal_stack_pages_node;
    free(new_signal_stack_pages_node);
  }

  // Restore spsr_el1 and elr_el1.

  __asm__ __volatile__("msr spsr_el1, %0" : : "r"(spsr_val));
  __asm__ __volatile__("msr elr_el1, %0" : : "r"(elr_val));

  // Restore the user SP.

  __asm__ __volatile__("msr sp_el0, %0" : : "r"(old_user_sp));

  // Restore the status bit.

  curr_thread->status.is_handling_signal = is_handling_signal;
}
