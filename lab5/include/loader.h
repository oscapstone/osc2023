#ifndef LOADER_H
#define LOADER_H
/// Run the program at the target location.
int run_program(void *);
void sys_run_program(void);
int setup_program_loc(void *);
void* getProgramLo();

// TODO: For future expansion.
int load_program();

#endif // LOADER_H
