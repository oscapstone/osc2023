#ifndef OSCOS_SCHED_H
#define OSCOS_SCHED_H

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdnoreturn.h>

#include "oscos/mem/types.h"
#include "oscos/uapi/signal.h"
#include "oscos/xcpt/trap-frame.h"

typedef struct thread_list_node_t {
  struct thread_list_node_t *prev, *next;
} thread_list_node_t;

typedef struct {
  alignas(16) uint64_t words[2];
} uint128_t;

typedef struct {
  uint128_t v[32];
  alignas(16) struct {
    uint64_t fpcr, fpsr;
  };
} thread_fp_simd_ctx_t;

typedef struct {
  alignas(16) union {
    struct {
      uint64_t r19, r20, r21, r22, r23, r24, r25, r26, r27, r28, r29, pc,
          kernel_sp, user_sp;
    };
    uint64_t regs[14];
  };
  thread_fp_simd_ctx_t *fp_simd_ctx;
} thread_ctx_t;

struct process_t;

typedef struct {
  thread_list_node_t list_node;
  thread_ctx_t ctx;
  size_t id;
  struct {
    bool is_waiting : 1;
    bool is_stopped : 1;
    bool is_waken_up_by_signal : 1;
    bool is_handling_signal : 1;
  } status;
  page_id_t stack_page_id;
  struct process_t *process;
} thread_t;

typedef struct page_id_list_node_t {
  page_id_t page_id;
  struct page_id_list_node_t *next;
} page_id_list_node_t;

typedef struct process_t {
  size_t id;
  shared_page_t *text_page;
#ifdef SCHED_ENABLE_SHARED_USER_STACK
  shared_page_t *user_stack_page;
#else
  page_id_t user_stack_page_id;
#endif
  page_id_list_node_t *signal_stack_pages;
  thread_t *main_thread;
  uint32_t pending_signals, blocked_signals;
  sighandler_t signal_handlers[32];
} process_t;

/// \brief Initializes the scheduler and creates the idle thread.
///
/// \return true if the initialization succeeds.
/// \return false if the initialization fails due to memory shortage.
bool sched_init(void);

/// \brief Creates a thread.
///
/// \param task The task to execute in the new thread.
/// \param arg The argument to pass to \p task.
/// \return true if the thread creation succeeds.
/// \return false if the thread creation fails due to memory shortage.
bool thread_create(void (*task)(void *), void *arg);

/// \brief Terminates the current thread.
///
/// This function should not be called on the idle thread.
noreturn void thread_exit(void);

/// \brief Gets the current thread.
thread_t *current_thread(void);

/// \brief Creates a process and name the current thread the main thread of the
///        process.
///
/// \return true if the process creation succeeds.
/// \return false if the process creation fails due to memory shortage.
bool process_create(void);

/// \brief Executes the first ever user program on the current process.
///
/// This function jumps to the user program and does not return if the operation
/// succeeds. If the operation fails due to memory shortage, this function
/// returns.
///
/// \param text_start The starting address of where the user program is
///                   currently located. (Do not confuse it with the entry point
///                   of the user program.)
/// \param text_len The length of the user program.
void exec_first(const void *text_start, size_t text_len);

/// \brief Executes a user program on the current process.
///
/// This function jumps to the user program and does not return if the operation
/// succeeds. If the operation fails due to memory shortage, this function
/// returns.
///
/// The text_page_id field of the current process must be valid.
///
/// \param text_start The starting address of where the user program is
///                   currently located. (Do not confuse it with the entry point
///                   of the user program.)
/// \param text_len The length of the user program.
void exec(const void *text_start, size_t text_len);

/// \brief Forks the current process.
process_t *fork(const extended_trap_frame_t *trap_frame);

/// \brief Kills zombie threads.
void kill_zombies(void);

/// \brief Yields CPU for the current thread and runs the scheduler.
void schedule(void);

/// \brief Puts the current thread to the given wait queue and runs the
///        scheduler.
void suspend_to_wait_queue(thread_list_node_t *wait_queue);

/// \brief Wake up every thread in the given wait queue.
void wake_up_all_threads_in_wait_queue(thread_list_node_t *wait_queue);

/// \brief Gets a process by its PID.
process_t *get_process_by_id(size_t pid);

/// \brief Kills a process.
void kill_process(process_t *process);

/// \brief Kills all processes.
void kill_all_processes(void);

/// \brief Sets up periodic scheduling.
void sched_setup_periodic_scheduling(void);

/// \brief Do what the idle thread should do.
noreturn void idle(void);

/// \brief Sets the signal handler of a process and returns the old one.
sighandler_t set_signal_handler(process_t *process, int signal,
                                sighandler_t handler);

/// \brief Delivers the given signal to the given process.
void deliver_signal(process_t *process, int signal);

/// \brief Delivers the given signal to all processes.
void deliver_signal_to_all_processes(int signal);

#endif
