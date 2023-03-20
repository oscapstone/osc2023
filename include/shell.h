#ifndef _SHELL_H
#define _SHELL_H
#define MAX_SHELL_INPUT 256
#define MAX_SHELL_OUTPUT 512
#define MAX_ARGS 63
extern void shell_process_cmd(char *input_buffer, unsigned int input_buffer_size);
extern int run_if_builtin(char *first_arg, char *other_args);
extern unsigned int shell_read_string(char *buffer, unsigned int buffer_size);
extern void shell_main(void);
extern int shell_parse_argv(char *s, char *argv[], unsigned max_args);
#endif