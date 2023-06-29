#include "oscos/xcpt/task-queue.h"

#include <limits.h>
#include <stdint.h>

#include "oscos/utils/critical-section.h"
#include "oscos/utils/heapq.h"

#define MAX_N_PENDING_TASKS 16

typedef struct {
  void (*task)(void *);
  void *arg;
  int priority;
} pending_task_t;

static pending_task_t _task_queue_pending_tasks[MAX_N_PENDING_TASKS];
static size_t _task_queue_n_pending_tasks = 0;
static int _task_queue_curr_task_priority = INT_MIN;

static int
_task_queue_pending_task_cmp_by_priority(const pending_task_t *const t1,
                                         const pending_task_t *const t2,
                                         void *const _arg) {
  (void)_arg;

  // Note that the order is reversed. This is because the heapq module
  // implements a min heap, but we want the task with the highest priority to be
  // at the front of the priority queue.

  if (t2->priority < t1->priority)
    return -1;
  if (t2->priority > t1->priority)
    return 1;
  return 0;
}

bool task_queue_add_task(void (*const task)(void *), void *const arg,
                         const int priority) {
  if (_task_queue_n_pending_tasks == MAX_N_PENDING_TASKS)
    return false;

  const pending_task_t entry = {.task = task, .arg = arg, .priority = priority};

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);
  heappush(_task_queue_pending_tasks, _task_queue_n_pending_tasks++,
           sizeof(pending_task_t), &entry,
           (int (*)(const void *, const void *,
                    void *))_task_queue_pending_task_cmp_by_priority,
           NULL);
  CRITICAL_SECTION_LEAVE(daif_val);

  return true;
}

void task_queue_sched(void) {
  const int curr_priority = _task_queue_curr_task_priority;

  while (_task_queue_n_pending_tasks > 0 &&
         _task_queue_pending_tasks[0].priority >
             curr_priority) { // There is a pending task of a higher priority.
    // Preempt the current task and run the highest-priority pending task.

    // Remove the highest-priority pending task from the priority queue.
    // No need to mask interrupts here; this function always runs within an ISR.

    pending_task_t entry;
    heappop(_task_queue_pending_tasks, _task_queue_n_pending_tasks--,
            sizeof(pending_task_t), &entry,
            (int (*)(const void *, const void *,
                     void *))_task_queue_pending_task_cmp_by_priority,
            NULL);

    // Update task priority.
    _task_queue_curr_task_priority = entry.priority;

    // Save spsr_el1 and elr_el1, since they can be clobbered.

    uint64_t spsr_val, elr_val;
    __asm__ __volatile__("mrs %0, spsr_el1" : "=r"(spsr_val));
    __asm__ __volatile__("mrs %0, elr_el1" : "=r"(elr_val));

    // Run the task with interrupts enabled.

    XCPT_UNMASK_ALL();
    entry.task(entry.arg);
    XCPT_MASK_ALL();

    // Restore spsr_el1 and elr_el1.
    __asm__ __volatile__("msr spsr_el1, %0" : : "r"(spsr_val));
    __asm__ __volatile__("msr elr_el1, %0" : : "r"(elr_val));
  }

  // Restore priority.
  _task_queue_curr_task_priority = curr_priority;
}
