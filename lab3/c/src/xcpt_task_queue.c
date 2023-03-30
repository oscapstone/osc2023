#include "oscos/xcpt_task_queue.h"

#include <limits.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdnoreturn.h>

#define TASK_STACK_AREA_SZ 8192
#define MAX_N_TASKS 8

typedef union {
  struct {
    uint64_t x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, sp, pstate,
        pc, x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14,
        x15, x16, x17, x18, x30;
  };
  uint64_t regs[34];
} xcpt_frame_t;

typedef struct {
  int priority;
  alignas(16) xcpt_frame_t ctx;
} task_t;

static task_t _tasks[MAX_N_TASKS] = {{INT_MIN, {{0}}}};
static size_t _n_tasks = 1;
static char _task_stacks[MAX_N_TASKS][TASK_STACK_AREA_SZ];
static size_t _curr_task = 0;

noreturn void xcpt_task_queue_run(void);
noreturn void xcpt_task_queue_fini(xcpt_frame_t *xcpt_frame);

bool task_queue_add(const int priority, void (*const task_fn)(void *),
                    void *const arg) {
  if (_n_tasks == MAX_N_TASKS)
    return false;

  const size_t i = _n_tasks++;
  _tasks[i] = (task_t){.priority = priority,
                       .ctx = {.x0 = (uint64_t)arg,
                               .x1 = i,
                               .x2 = (uint64_t)task_fn,
                               .sp = (uint64_t)_task_stacks[i],
                               .pstate = 0x5, // EL1h. All interrupts unmasked.
                               .pc = (uint64_t)xcpt_task_queue_run}};

  return true;
}

void task_queue_remove(const size_t i) { _tasks[i] = _tasks[--_n_tasks]; }

noreturn void task_queue_sched(void) {
  _curr_task = 0;
  for (size_t i = 1; i < _n_tasks; i++) {
    if (_tasks[i].priority > _tasks[_curr_task].priority) {
      _curr_task = i;
    }
  }

  xcpt_task_queue_fini(&_tasks[_curr_task].ctx);
}

void task_queue_entry(xcpt_frame_t *const xcpt_frame) {
  _tasks[_curr_task].ctx = *xcpt_frame;
  task_queue_sched();
}
