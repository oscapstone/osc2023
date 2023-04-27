#include "../include/sched.h"
#include "mm.h"
#include "exception.h"
#include "mini_uart.h"

static struct task_struct init_task = INIT_TASK;
struct task_struct *current = &(init_task);
struct task_struct *task[NR_TASKS] = {&(init_task), };  //NR_TASKS = 64
int nr_tasks = 1;

void preempt_disable() {
    current->preempt_count++;
}

void preempt_enable() {
    current->preempt_count--;
}

void _schedule(){
    int next, c;
    struct task_struct *p;
    while(1){
        c=-1;
        next=0;
        for (int i=0;i<NR_TASKS;i++){
            if(task[i]==NULL) continue;
            p=task[i];
            if(p&&p->state == TASK_RUNNING && p->counter > c){
                c=p->counter;
                next=i;
            }
        }
        if(c){
            //no task waiting
            break;
        }
        //update the priority of tasks
        for(int i=0;i<NR_TASKS;i++){
            if(task[i]==NULL) continue;
            p=task[i];
            if(p){
                p->counter =(p->counter >> 1) + p->priority;        //counter/2 +priority
            }
        }
    }
    preempt_disable();
    switch_to(task[next]);
    preempt_enable();
}

void schedule(){
    current->counter =0 ;   //current state won't be selected again unless there're no task waiting
    _schedule();
}

void switch_to(struct task_struct *next){
    if(current == next) return;
    
    struct task_struct *prev=current;
    current=next;
    cpu_switch_to(prev,next);
}

void timer_tick(){
    --current->counter;
    if(current->counter>0 || current->preempt_count>0) return;

    current->counter=0;
    enable_interrupt();
    _schedule();
    disable_interrupt();
}

void schedule_tail() {
    preempt_enable();
}

void exit_process(){
    //should only be accessed using syscall
    current->state=TASK_ZOMBIE;     //prevents the task from being selected and executed by the scheduler.
    /*ZOMBIE:In Linux such approach is used to allow parent process to query information about the child even after it finishes.*/
    
    free((void*)current->stack);
    schedule();         //new task will be selected
}

void kill_zombies(){

    struct task_struct *p;
    for(int i=0;i<NR_TASKS;i++){
        if(task[i==NULL]) continue;

        p=task[i];
        if(p && p->state == TASK_ZOMBIE){
            printf("Zombie found with pid: %d.\n", p->id);
            free(p);
            task[i] = NULL;
        }
    }
}