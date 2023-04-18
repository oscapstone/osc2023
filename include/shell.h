#ifndef _SHELL_H
#define _SHELL_H
#include "stddef.h"
#define MAX_SHELL_INPUT 256
#define MAX_SHELL_OUTPUT 512
#define MAX_ARGS 63
typedef void (*shell_builtin_func)(int argc, char *argv[]);
struct shell_cmd {
    char *name;
    char *help;
    shell_builtin_func func;
};
extern struct shell_cmd cmd_list[];
extern size_t shell_cmd_cnt;
extern void shell_process_cmd(char *input_buffer, unsigned int input_buffer_size);
extern int run_if_builtin(int argc, char *argv[]);
extern unsigned int shell_read_string(char *buffer, unsigned int buffer_size);
extern void shell_main(void);
extern void shell_main_thread(void);
extern int shell_parse_argv(char *s, char *argv[], unsigned max_args);
#endif