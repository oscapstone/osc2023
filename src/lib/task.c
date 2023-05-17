#include <task.h>
#include <mm.h>
#include <signal.h>

static struct list_head task_queue;

uint32 max_tid;

static uint32 alloc_tid(void){
    uint32 tid;

    tid = max_tid;
    max_tid += 1;

    return tid;
}

void task_init(void){
    INIT_LIST_HEAD(&task_queue);
}

task_struct *task_create(void){
    task_struct *task;
    struct signal_head_t *signal;
    struct sighand_t *sighand;

    task = kmalloc(sizeof(task_struct));
    signal = signal_head_create();
    sighand = sighand_create();

    task->kernel_stack = NULL;
    task->user_stack = NULL;
    task->data = NULL;
    INIT_LIST_HEAD(&task->list);
    list_add_tail(&task->task_list, &task_queue);
    task->status = TASK_NEW;
    task->need_resched = 0;
    task->tid = alloc_tid();
    task->preempt = 0;

    task->signal = signal;
    task->sighand = sighand;

    return task;
}

void task_free(task_struct *task){
    if(task->kernel_stack)
        kfree(task->kernel_stack);
    
    if (task->user_stack)
        kfree(task->user_stack);

    if (task->data)
        kfree(task->data);

    list_del(&task->task_list);

    signal_head_free(task->signal);
    sighand_free(task->sighand);

    kfree(task);
}

task_struct *task_get_by_tid(uint32 tid){
    task_struct *task;

    list_for_each_entry(task, &task_queue, task_list) {
        if (task->tid == tid) {
            return task;
        }
    }

    return NULL;
}