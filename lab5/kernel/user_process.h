#ifndef USER_PROCESS_C
#define USER_PROCESS_C

void init_user_process(void);
int get_pid(void);
void set_sp_on_exception(char *val);

void exit_current_process(void);
void kill_process(unsigned int id);
int fork_process(int pid);
int replace_process(int pid, const char *name, char *const argv[]);

void shell_exec(void);

void demo_user_proc(void);

#endif /* USER_PROCESS_C */