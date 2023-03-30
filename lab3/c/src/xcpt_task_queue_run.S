.section ".text"

xcpt_task_queue_run:
    mov x19, x1
    blr x2

    mov x0, x19
    bl task_queue_remove

    msr daifset, 0xf
    b task_queue_sched

.size xcpt_task_queue_run, . - xcpt_task_queue_run
.type xcpt_task_queue_run, function
.global xcpt_task_queue_run
