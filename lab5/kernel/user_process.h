#ifndef USER_PROCESS_C
#define USER_PROCESS_C

int check_preemptable(void);
void enable_preemption(void);
void disable_preemption(void);
int running_user_process(void);

void init_user_process(void);
int get_pid(void);
void* return_value_ptr(void);

void exit_current_process(void);
void kill_process(unsigned int id);
int fork_process(int pid);
int replace_process(int pid, const char *name, char *const argv[]);

void shell_exec(void);
void create_and_execute_process(void *prog_ptr);

void demo_user_proc(void);

#endif /* USER_PROCESS_C */