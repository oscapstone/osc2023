#include "mini_uart.h"
#include "mem_frame.h"
#include "utils.h"

#define MAX_NUM_THREAD 64
#define NULL (void*)0xFFFFFFFFFFFFFFFF

struct collee_saved_register {
        unsigned long x19;
        unsigned long x20;
        unsigned long x21;
        unsigned long x22;
        unsigned long x23;
        unsigned long x24;
        unsigned long x25;
        unsigned long x26;
        unsigned long x27;
        unsigned long x28;
};

struct context {
        struct collee_saved_register reg;
        unsigned long fp;
        unsigned long lr;
        unsigned long sp;

        struct context *next;

        unsigned int id;
        unsigned int dead;
};
struct context* threads[MAX_NUM_THREAD];

struct context *ready_q_front, *ready_q_end;

extern void switch_context(void *prev_ptr, void *next_ptr);
extern struct context* get_current_thread(void);

void init_thread(void)
{
        for (int i = 0; i < MAX_NUM_THREAD; i++) {
                threads[i] = NULL;
        }

        struct context *new_thread = allocate_frame(0);
        new_thread->id = 0;
        new_thread->next = NULL;
        new_thread->dead = 0;
        threads[0] = new_thread;

        asm volatile("msr tpidr_el1, %0" : : "r"(new_thread));

        ready_q_front = NULL;
        ready_q_end = NULL;
}

void kill_zombies(void)
{
        // kill in schedule()
}

void schedule(void)
{
        if (ready_q_front == NULL) {
                asm volatile("ldr x0, =_start");
                asm volatile("mov sp, x0");
                asm volatile("bl shell_loop");
        }

        struct context *next_thread = ready_q_front;
        ready_q_front = next_thread->next;
        next_thread->next = NULL;

        struct context *current_thread = get_current_thread();
        if (current_thread->dead) {
                threads[current_thread->id] = NULL;
                free_frame(current_thread);
        } else if (ready_q_front == NULL) {
                ready_q_front = current_thread;
                ready_q_end = current_thread;
        } else {
                ready_q_end->next = current_thread;
                ready_q_end = current_thread;
        }
        switch_context(current_thread, next_thread);
}

void thread_wrapper(void)
{
        void (*thread_func)(void);
        asm volatile("mov %0, x19" : "=r"(thread_func));
        thread_func();

        struct context *current_thread = get_current_thread();
        current_thread->dead = 1;

        schedule();
        // TODO: return if all thread is done?
        while (1) {}
}

/*
 * Returns id of created thread
 */
int thread_create(void (*thread_func)(void))
{
        int id;
        for (id = 0; id < MAX_NUM_THREAD && threads[id] != NULL; id++);
        if (id >= MAX_NUM_THREAD) {
                uart_send_string("[ERROR] exceed max number of thread\r\n");
                while (1) {}
                return -1;
        }

        struct context *new_thread = allocate_frame(0);
        new_thread->reg.x19 = (unsigned long)thread_func;
        new_thread->lr = (unsigned long)thread_wrapper;
        new_thread->sp = (unsigned long)new_thread + FRAME_SIZE;
        new_thread->next = NULL;
        new_thread->id = id;
        new_thread->dead = 0;
        threads[id] = new_thread;

        if (ready_q_front == NULL) {
                ready_q_front = new_thread;
        } else {
                ready_q_end->next = new_thread;
        }
        ready_q_end = new_thread;

        return id;
}

void idle(void)
{
        while (1) {
                kill_zombies();
                schedule();
        }
}

/////////////////////////////////////////////////////////
//   DEMO                                              //
/////////////////////////////////////////////////////////

void foo(void)
{
        for(int i = 0; i < 10; i++) {
                uart_send_string("Thread id: ");
                uart_send_int(get_current_thread()->id);
                uart_send_string(", ");
                uart_send_int(i);
                uart_endl();
                delay(1000000);
                schedule();
        }
}

void demo_thread(int num_thread)
{
        for (int i = 0; i < num_thread; i++) {
                thread_create(foo);
        }
        idle();
}