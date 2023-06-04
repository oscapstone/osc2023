#ifndef OSCOS_SCHED_H
#define OSCOS_SCHED_H

#include <stdalign.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdnoreturn.h>

#include "oscos/mem/types.h"

typedef struct thread_list_node_t {
  struct thread_list_node_t *prev, *next;
} thread_list_node_t;

typedef struct {
  alignas(16) union {
    struct {
      uint64_t x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, pc, sp;
    };
    uint64_t regs[13];
  };
} thread_ctx_t;

typedef struct {
  thread_list_node_t list_node;
  thread_ctx_t ctx;
  size_t id;
  page_id_t stack_page_id;
} thread_t;

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

/// \brief Kills zombie threads.
void kill_zombies(void);

/// \brief Yields CPU for the current thread and runs the scheduler.
void schedule(void);

/// \brief Do what the idle thread should do.
///
/// This function must be called on the idle thread, i.e., the thread that
/// called bool thread_create(void (*)(void *), void *).
noreturn void idle(void);

#endif
