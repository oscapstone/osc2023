#include "mini_uart.h"
#include "mem_frame.h"
#include "mem_utils.h"
#include "exception.h"
#include "ramdisk.h"
#include "timer.h"

#include "syscall.h"
#include "utils.h"

#define MAX_NUM_PROCESS 64
#define PROCESS_SIZE (4 * FRAME_SIZE)
#define PROCESS_SIZE_ORDER 2
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

struct el0_context {
        struct collee_saved_register el1_reg;
        unsigned long fp;
        unsigned long lr;
        unsigned long sp;

        unsigned long elr_el1;
        unsigned long sp_el0;
        // unsigned long esr_el1;

        struct el0_context *next;

        unsigned int id;
        unsigned int dead;
};
struct el0_context* processes[MAX_NUM_PROCESS];
unsigned long return_value_offset;

struct el0_context *p_ready_q_front, *p_ready_q_end;

extern struct el0_context* get_current_process(void);
extern void set_current_process(struct el0_context* ptr);
extern void user_proc_context_switch
                (struct el0_context *prev, struct el0_context *next);
void load_process_context(struct el0_context *ptr_context);

static int preemption_enabled, executing_user_process;

int check_preemptable(void)
{
        return preemption_enabled;
}

void enable_preemption(void)
{
        preemption_enabled = 1;
}

void disable_preemption(void)
{
        preemption_enabled = 0;
}

void* return_value_ptr(void)
{
        return get_current_process() + return_value_offset;
}

int running_user_process(void)
{
        return executing_user_process;
}

void init_user_process(void)
{
        for (int i = 0; i < MAX_NUM_PROCESS; i++) {
                processes[i] = NULL;
        }

        set_current_process((struct el0_context*)NULL);

        p_ready_q_front = NULL;
        p_ready_q_end = NULL;

        return_value_offset = FRAME_SIZE - 16 * 16;
}

int get_pid(void)
{
        return get_current_process()->id;
}

// TODO: load to a newly allocated memory space and add systemcall exit to it?
int process_create(void *prog_ptr)
{
        int id;
        for (id = 0; id < MAX_NUM_PROCESS && processes[id] != NULL; id++);
        if (id >= MAX_NUM_PROCESS) {
                uart_send_string("[ERROR] exceed max number of process\r\n");
                while (1) {}
                return -1;
        }

        struct el0_context *new_process = allocate_frame(PROCESS_SIZE_ORDER);
        new_process->sp = (unsigned long)new_process + return_value_offset;
        new_process->elr_el1 = (unsigned long)prog_ptr;
        new_process->sp_el0 = (unsigned long)new_process + PROCESS_SIZE;

        new_process->next = NULL;
        new_process->id = id;
        new_process->dead = 0;

        processes[id] = new_process;

        if (p_ready_q_front == NULL) {
                p_ready_q_front = new_process;
        } else {
                p_ready_q_end->next = new_process;
        }
        p_ready_q_end = new_process;

        return id;
}

struct el0_context* choose_next_process(void)
{
        struct el0_context *current_process = get_current_process();
        if (current_process != NULL) {
                // Puts it to the end of ready queue
                if (p_ready_q_front == NULL) {
                        p_ready_q_front = current_process;
                } else {
                        p_ready_q_end->next = current_process;
                }
                p_ready_q_end = current_process;
        }

        struct el0_context *new_process;
        while (1) {
                if (p_ready_q_front == NULL) {
                        // Returns to shell
                        executing_user_process = 0;
                        // TODO(lab5): disable timer
                        // disable_interrupts_in_el1();
                        // reset_core_timer_in_second(600);
                        asm volatile("ldr x0, =_start");
                        asm volatile("mov sp, x0");
                        asm volatile("bl shell_loop");
                } else if (p_ready_q_front->dead) {
                        // Free this process
                        struct el0_context *to_free = p_ready_q_front;
                        p_ready_q_front = to_free->next;
                        processes[to_free->id] = NULL;
                        free_frame(to_free);
                } else {
                        // Found a new process
                        new_process = p_ready_q_front;
                        p_ready_q_front = new_process->next;
                        new_process->next = NULL;
                        break;
                }
        }
        return new_process;
}

void exit_current_process(void)
{
        struct el0_context *current_process = get_current_process();
        current_process->dead = 1;

        struct el0_context *next_process = choose_next_process();
        load_process_context(next_process); // TODO
}

void kill_process(unsigned int id)
{
        if (processes[id] == NULL) {
                uart_send_string("[ERROR] kill process: invalid id\r\n");
                return;
        }

        struct el0_context *current_process = get_current_process();
        if (current_process->id == id) exit_current_process();

        processes[id]->dead = 1;
}

void create_and_execute_process(void *prog_ptr)
{       
        process_create(prog_ptr);
        struct el0_context *next_process = choose_next_process();
        set_current_process(next_process);

        executing_user_process = 1;
        // TODO(lab5): enable timer
        // reset_core_timer_in_second(1);
        // enable_interrupts_in_el1();
        asm volatile("mov x2, 0x340");
        asm volatile("msr spsr_el1, x2");
        asm volatile("msr elr_el1, %0" : : "r"(next_process->elr_el1));
        asm volatile("msr sp_el0, %0" : : "r"(next_process->sp_el0));
        asm volatile("mov sp, %0" : : "r"(next_process->sp));
        asm volatile("eret");
}

struct el0_context* copy_process(struct el0_context* process_ptr)
{
        int child_id = process_create(0);
        struct el0_context *child_ptr = processes[child_id];

        struct el0_context *child_next = child_ptr->next;

        memcpy(child_ptr, process_ptr, PROCESS_SIZE);
        child_ptr->fp = process_ptr->fp - (unsigned long)process_ptr
                                        + (unsigned long)child_ptr;
        child_ptr->sp = process_ptr->sp - (unsigned long)process_ptr
                                        + (unsigned long)child_ptr;
        child_ptr->sp_el0 = process_ptr->sp_el0 - (unsigned long)process_ptr
                                                + (unsigned long)child_ptr;
        child_ptr->id = child_id;
        child_ptr->next = child_next;

        return child_ptr;
}

int fork_process(int pid)
{
        struct el0_context *process_ptr = processes[pid];
        struct el0_context *child_ptr = copy_process(process_ptr);
        *((unsigned long*)child_ptr + return_value_offset) = 0;
        return child_ptr->id;
}

int replace_process(int pid, char *name, char *const argv[])
{
        char *program_adr = ramdisk_find_file(name);
        if (program_adr == NULL) {
                uart_send_string("[ERROR] syscall exec: failed to load\r\n");
                return -1;
        }

        struct el0_context *process_ptr = processes[pid];
        process_ptr->sp = (unsigned long)process_ptr + return_value_offset;
        process_ptr->elr_el1 = (unsigned long)program_adr;
        process_ptr->sp_el0 = (unsigned long)process_ptr + PROCESS_SIZE;

        return 0;
}

void preemption(void)
{
        if (!preemption_enabled && executing_user_process) return;

        struct el0_context *current_process = get_current_process();
        unsigned int dead = current_process -> dead;
        struct el0_context *next_process = choose_next_process();
        if (current_process == next_process) return;

        if (dead) {
                load_process_context(next_process);
        } else {
                user_proc_context_switch(current_process, next_process);
        }
}

void shell_exec(void)
{
        char filename[100];
        uart_send_string("Filename: ");
        uart_readline(filename, 100);
        char *program_adr = ramdisk_find_file(filename);
        if (program_adr == NULL) {
                uart_send_string("[ERROR] exec: failed to load\r\n");
                return;
        }
        create_and_execute_process(program_adr);
}

/////////////////////////////////////////////////////////
//   DEMO                                              //
/////////////////////////////////////////////////////////

void fork_test(){
        uart_send_string("\r\n Fork Test, pid ");
        uart_send_int(getpid());
        uart_endl();

        int cnt = 1;
        int ret = 0;
        if ((ret = fork()) == 0) { // child
                long long cur_sp;
                asm volatile("mov %0, sp" : "=r"(cur_sp));
                uart_send_string("first child pid: ");
                uart_send_int(getpid());
                uart_send_string(", cnt: ");
                uart_send_int(cnt);
                uart_send_string(", ptr: ");
                uart_send_hex_64((unsigned long)&cnt);
                uart_send_string(", sp: ");
                uart_send_hex_64((unsigned long)&cur_sp);
                uart_endl();

                ++cnt;

                if ((ret = fork()) != 0) {
                        asm volatile("mov %0, sp" : "=r"(cur_sp));
                        uart_send_string("first child pid: ");
                        uart_send_int(getpid());
                        uart_send_string(", cnt: ");
                        uart_send_int(cnt);
                        uart_send_string(", ptr: ");
                        uart_send_hex_64((unsigned long)&cnt);
                        uart_send_string(", sp: ");
                        uart_send_hex_64((unsigned long)&cur_sp);
                        uart_endl();
                }
                else{
                        while (cnt < 5) {
                                asm volatile("mov %0, sp" : "=r"(cur_sp));
                                uart_send_string("second child pid: ");
                                uart_send_int(getpid());
                                uart_send_string(", cnt: ");
                                uart_send_int(cnt);
                                uart_send_string(", ptr: ");
                                uart_send_hex_64((unsigned long)&cnt);
                                uart_send_string(", sp: ");
                                uart_send_hex_64((unsigned long)&cur_sp);
                                uart_endl();

                                delay(1000000);
                                ++cnt;
                        }
                }
                exit(0);
        }
        else {
                uart_send_string("parent here, pid ");
                uart_send_int(getpid());
                uart_send_string(", child: ");
                uart_send_int(ret);
                uart_endl();
        }
        exit(0); // TODO(lab5): how to exit without this??
}

void foo_user_02(void)
{
        uart_send_string("FOO 2 IS HERE\r\n");
        exit(0);
}

void foo_user_01(void)
{
        uart_send_string("FOO 1 IS HERE\r\n");
        uart_send_int(getpid());
        uart_endl();
        int ret = 0;
        if ((ret = fork()) == 0) {
                // child
                uart_send_string("child is here: ");
                uart_send_int(getpid());
                uart_endl();
        } else {
                // parent
                uart_send_string("parent is here, child = ");
                uart_send_int(ret);
                uart_endl();
        }
        int ret2 = fork();
        kill(ret2);
        uart_send_string("FOO 1 DONE\r\n");
        exit(0);
        // uart_send_string("enter user process\r\n");
        // while (1) {}
}

void demo_user_proc(void)
{
        create_and_execute_process(foo_user_01);
        // create_and_execute_process(fork_test);
}