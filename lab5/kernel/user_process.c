#include "mini_uart.h"
#include "mem_frame.h"
#include "mem_utils.h"
#include "exception.h"
#include "ramdisk.h"

#include "syscall.h"
#include "utils.h"

#define MAX_NUM_PROCESS 64
#define PROCESS_SIZE (4 * FRAME_SIZE)
#define PROCESS_SIZE_ORDER 2
#define NULL (void*)0xFFFFFFFFFFFFFFFF

struct context {
        unsigned long reg[31];
        unsigned long lr;
        unsigned long sp;

        struct context *next;

        unsigned int id;
        unsigned int dead;
};
struct context* processes[MAX_NUM_PROCESS];

struct context *p_ready_q_front, *p_ready_q_end;

static char *sp_on_exception;

extern struct context* get_current_process(void);
extern void set_current_process(struct context* ptr);

void set_sp_on_exception(char *val)
{
        sp_on_exception = val;
}

void init_user_process(void)
{
        for (int i = 0; i < MAX_NUM_PROCESS; i++) {
                processes[i] = NULL;
        }

        set_current_process((struct context*)NULL);

        p_ready_q_front = NULL;
        p_ready_q_end = NULL;
}

int get_pid(void)
{
        return get_current_process()->id;
}

// TODO: load to a newly allocated memory space and add systemcall exit to it
int process_create(void *prog_ptr)
{
        int id;
        for (id = 0; id < MAX_NUM_PROCESS && processes[id] != NULL; id++);
        if (id >= MAX_NUM_PROCESS) {
                uart_send_string("[ERROR] exceed max number of process\r\n");
                while (1) {}
                return -1;
        }

        struct context *new_process = allocate_frame(PROCESS_SIZE_ORDER);
        new_process->lr = (unsigned long)prog_ptr;
        new_process->sp = (unsigned long)new_process + PROCESS_SIZE;
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

void save_el0_context(void)
{
        struct context *ptr_context = get_current_process();
        memcpy(ptr_context, sp_on_exception, 31 * sizeof(unsigned long));

        unsigned long lr, sp;
        asm volatile("mrs %0, elr_el1" : "=r"(lr));
        asm volatile("mrs %0, sp_el0" : "=r"(sp));
        ptr_context->lr = lr;
        ptr_context->sp = sp;
}

void load_el0_context(struct context *ptr_context)
{
        memcpy(sp_on_exception, ptr_context, 31 * sizeof(unsigned long));

        unsigned long lr, sp;
        lr = ptr_context->lr;
        asm volatile("msr elr_el1, %0" : "=r"(lr));
        sp = ptr_context->sp;
        asm volatile("msr sp_el0, %0" : "=r"(sp));

        set_current_process(ptr_context);
}

struct context* choose_next_process(void)
{
        struct context *new_process;
        while (1) {
                if (p_ready_q_front == NULL) {
                        asm volatile("ldr x0, =_start");
                        asm volatile("mov sp, x0");
                        asm volatile("bl shell_loop");
                } else if (p_ready_q_front->dead) {
                        struct context *to_free = p_ready_q_front;
                        p_ready_q_front = to_free->next;
                        processes[to_free->id] = NULL;
                        free_frame(to_free);
                } else {
                        break;
                }
        }
        new_process = p_ready_q_front;
        p_ready_q_front = new_process->next;
        new_process->next = NULL;

        struct context *current_process = get_current_process();
        if (current_process == NULL) return new_process;

        if (p_ready_q_front == NULL) {
                p_ready_q_front = current_process;
                p_ready_q_end = current_process;
        } else {
                p_ready_q_end->next = current_process;
                p_ready_q_end = current_process;
        }

        return new_process;
}

void exit_current_process(void)
{
        struct context *current_process = get_current_process();
        current_process->dead = 1;

        struct context *next_process = choose_next_process();
        load_el0_context(next_process);
}

void kill_process(unsigned int id)
{
        if (processes[id] == NULL) {
                uart_send_string("[ERROR] kill process: invalid id\r\n");
                return;
        }

        struct context *current_process = get_current_process();
        if (current_process->id == id) exit_current_process();

        processes[id]->dead = 1;
}

int create_and_execute(void *prog_ptr)
{
        int id = process_create(prog_ptr);

        struct context *next_process = choose_next_process();
        set_current_process(next_process);
        branch_to_address_el0(prog_ptr, (char*)(next_process->sp));

        return id;
}

struct context* copy_process(struct context* process_ptr)
{
        int child_id = process_create(0);
        struct context *child_ptr = processes[child_id];

        struct context *child_next = child_ptr->next;

        memcpy(child_ptr, process_ptr, PROCESS_SIZE);
        child_ptr->next = child_next;
        child_ptr->id = child_id;
        child_ptr->sp = process_ptr->sp - (unsigned long)process_ptr
                                        + (unsigned long)child_ptr;

        return child_ptr;
}

int fork_process(int pid)
{
        save_el0_context();
        struct context *process_ptr = processes[pid];
        struct context *child_ptr = copy_process(process_ptr);
        child_ptr->reg[0] = 0;
        return child_ptr->id;
}

int replace_process(int pid, char *name, char *const argv[])
{
        char *program_adr = ramdisk_find_file(name);
        if (program_adr == NULL) {
                uart_send_string("[ERROR] syscall exec: failed to load\r\n");
                return -1;
        }
        asm volatile("msr elr_el1, %0" : "=r"(program_adr));

        char *stack = (char*)processes[pid] + PROCESS_SIZE;
        asm volatile("msr sp_el0, %0" : "=r"(stack));

        return 0;
}

void shell_exec(void)
{
        // TODO: enable preemption
        // reset_core_timer_in_second(2);
        char filename[100];
        uart_send_string("Filename: ");
        uart_readline(filename, 100);
        char *program_adr = ramdisk_find_file(filename);
        if (program_adr == NULL) {
                uart_send_string("[ERROR] exec: failed to load\r\n");
                return;
        }
        create_and_execute(program_adr);
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
        exit(0); // TODO: how to exit without this??
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
}

void demo_user_proc(void)
{
        // create_and_execute(foo_user_01);
        create_and_execute(fork_test);
}