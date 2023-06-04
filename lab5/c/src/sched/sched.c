// This module implements a round-robin scheduler.

#include "oscos/sched.h"

#include "oscos/mem/malloc.h"
#include "oscos/mem/page-alloc.h"
#include "oscos/mem/vm.h"
#include "oscos/utils/critical-section.h"

#define THREAD_STACK_ORDER 13 // 8KB.
#define THREAD_STACK_BLOCK_ORDER (THREAD_STACK_ORDER - PAGE_ORDER)

void _sched_run_thread(thread_t *thread);
void thread_main();

static size_t _sched_next_tid = 1;
static thread_list_node_t _run_queue, _zombies;

bool sched_init(void) {
  // Creates the idle thread.

  thread_t *const idle_thread = malloc(sizeof(thread_t));
  if (!idle_thread)
    return false;

  idle_thread->id = 0;

  // Initializes the run queue. Initially, the run queue consists solely of the
  // idle thread.

  _run_queue.prev = _run_queue.next = &idle_thread->list_node;
  idle_thread->list_node.prev = idle_thread->list_node.next = &_run_queue;

  // Name the current thread the idle thread.

  __asm__ __volatile__("msr tpidr_el1, %0" : : "r"(idle_thread));

  // Initialize the zombie list. Initially, there are no zombies.

  _zombies.prev = _zombies.next = &_zombies;

  return true;
}

bool thread_create(void (*const task)(void *), void *const arg) {
  // Create the thread structure.

  thread_t *const thread = malloc(sizeof(thread_t));
  if (!thread)
    return false;

  thread->id = _sched_next_tid++;
  thread->ctx.x19 = (uint64_t)(uintptr_t)task;
  thread->ctx.x20 = (uint64_t)(uintptr_t)arg;
  thread->ctx.pc = (uint64_t)(uintptr_t)thread_main;

  // Allocate the stack for the new thread.

  const spage_id_t stack_page_id = alloc_pages(THREAD_STACK_BLOCK_ORDER);
  if (stack_page_id < 0) {
    free(thread);
    return false;
  }

  thread->stack_page_id = stack_page_id;
  void *const init_sp = (char *)pa_to_kernel_va(page_id_to_pa(stack_page_id)) +
                        (1 << (THREAD_STACK_BLOCK_ORDER + PAGE_ORDER));
  thread->ctx.sp = (uint64_t)(uintptr_t)init_sp;

  // Put the thread into the end of the run queue.

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  thread_list_node_t *const last_node = _run_queue.prev;
  thread->list_node.prev = last_node;
  last_node->next = &thread->list_node;
  thread->list_node.next = &_run_queue;
  _run_queue.prev = &thread->list_node;

  CRITICAL_SECTION_LEAVE(daif_val);

  return true;
}

void thread_exit(void) {
  // Move the current thread from the run queue to the zombie list.

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  thread_list_node_t *const current_thread_node = _run_queue.next,
                            *const next_node = current_thread_node->next;
  _run_queue.next = next_node;
  next_node->prev = &_run_queue;

  thread_list_node_t *const last_zombie = _zombies.prev;
  current_thread_node->prev = last_zombie;
  last_zombie->next = current_thread_node;
  current_thread_node->next = &_zombies;
  _zombies.prev = current_thread_node;

  CRITICAL_SECTION_LEAVE(daif_val);

  // Pick a new thread to run.

  thread_t *const next_thread =
      (thread_t *)((char *)next_node - offsetof(thread_t, list_node));
  _sched_run_thread(next_thread);
  __builtin_unreachable();
}

thread_t *current_thread(void) {
  thread_t *result;
  __asm__ __volatile__("mrs %0, tpidr_el1" : "=r"(result));
  return result;
}

static void _thread_cleanup(thread_t *const thread) {
  free_pages(thread->stack_page_id);
  free(thread);
}

void kill_zombies(void) {
  for (;;) {
    uint64_t daif_val;
    CRITICAL_SECTION_ENTER(daif_val);

    if (_zombies.next == &_zombies) { // No more zombies.
      CRITICAL_SECTION_LEAVE(daif_val);
      break;
    }

    thread_list_node_t *const first_zombie_node = _zombies.next,
                              *const next_zombie_node = first_zombie_node->next;

    _zombies.next = next_zombie_node;
    next_zombie_node->prev = &_zombies;

    CRITICAL_SECTION_LEAVE(daif_val);

    thread_t *const zombie =
        (thread_t *)((char *)first_zombie_node - offsetof(thread_t, list_node));
    _thread_cleanup(zombie);
  }
}

thread_t *_sched_pick_thread(void) {
  // Move the current thread to the end of the run queue.

  uint64_t daif_val;
  CRITICAL_SECTION_ENTER(daif_val);

  thread_list_node_t *const current_thread_node = _run_queue.next,
                            *const next_node = current_thread_node->next;
  _run_queue.next = next_node;
  next_node->prev = &_run_queue;

  thread_list_node_t *const last_node = _run_queue.prev;
  current_thread_node->prev = last_node;
  last_node->next = current_thread_node;
  current_thread_node->next = &_run_queue;
  _run_queue.prev = current_thread_node;

  thread_t *const next_thread =
      (thread_t *)((char *)_run_queue.next - offsetof(thread_t, list_node));

  CRITICAL_SECTION_LEAVE(daif_val);
  return next_thread;
}
