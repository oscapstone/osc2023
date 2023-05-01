#include "dtb.h"
#include "uart.h"
#include "utils.h"
#include "reboot.h"
#include "shell.h"
#include "note.h"
#include "thread.h"
#include "initramfs.h"
#include "time_interrupt.h"
#include "syscall.h"

static void hello_cmd(int argc, char *argv[])
{
    uart_write_string("Hello World!\n");
}

static void reboot_cmd(int argc, char *argv[])
{
    reset(0);
    while (1) delay(10);
}
static void ls_cmd(int argc, char *argv[])
{
    _initramfs.ls(&_initramfs, "");
}

static void cat_cmd(int argc, char *argv[])
{
    if (argc < 2) {
        uart_write_string("Usage: cat <file name>\n");
        return;
    }
    _initramfs.cat(&_initramfs, argv[1]);
}
static void dump_cmd(int argc, char *argv[])
{
    if (argc < 2 || strlen(argv[1]) != 16) {
        uart_write_string("Usage: dump <64b hex address>\n");
        return;
    }
    dump_hex((void *)hex2ull(argv[1]), 8);
}
static void mknote_cmd(int argc, char *argv[])
{
    make_note();
}
static void linote_cmd(int argc, char *argv[])
{
    list_note();
}
static void dtb_cmd(int argc, char *argv[])
{
    _fdt.fdt_print(&_fdt);
}
static void ldprog_cmd(int argc, char *argv[])
{
    if (argc < 2) {
        uart_write_string("Usage: ldprog <file name>\n");
        return;
    }
    _initramfs.exec(&_initramfs, &argv[1]);
}
static void uptime_cmd(int argc, char *argv[])
{
    write_uptime();
}
static void async_cmd(int argc, char *argv[])
{
    test_uart_async();
}
static void timeout_cmd(int argc, char *argv[])
{
    char input_buffer[MAX_SHELL_INPUT];
    uart_write_string("message length: ");
    uart_read_input(input_buffer, MAX_SHELL_INPUT);
    int buf_size = atoi(input_buffer);
    if (buf_size <= 0 || buf_size > 100) {
        uart_write_string("length should within [1, 100].\n");
        return;
    }
    char *msg = (char *)simple_malloc(buf_size * sizeof(char));
    uart_write_string("message: ");
    uart_read_input(msg, buf_size);
    uart_write_string("duration: ");
    uart_read_input(input_buffer, MAX_SHELL_INPUT);
    int dur = atoi(input_buffer);
    if (dur <= 0 || dur > 100) {
        uart_write_string("duration should within [1, 100].\n");
        return;
    }
    _timer_task_scheduler.add_timer_second(&_timer_task_scheduler, notify, msg, dur);
}
static void preempt_cmd(int argc, char *argv[])
{
    _timer_task_scheduler.add_timer_second(&_timer_task_scheduler, sleep_timer, 1, 1);
}
static void malloc_cmd(int argc, char *argv[])
{
    test_mem_pool();
}
static void buddy_cmd(int argc, char *argv[])
{
    test_buddy();
}
static void thread_cmd(int argc, char *argv[])
{
    demo_thread();
}

static void demo_fork_test()
{
    run_user_prog(fork_test);
    // _exit(0);
}

static void fork_cmd(int argc, char *argv[])
{
    // fork_test();
    create_thread(demo_fork_test);
}

static void kill_shell()
{
    kill(1);
}

static void demo_kill()
{
    run_user_prog(kill_shell);
}

static void kill_cmd(int argc, char *argv[])
{
    //kill shell
    create_thread(demo_kill);
}

static void help_cmd(int argc, char *argv[])
{
    for (size_t i = 0; i < shell_cmd_cnt; i++) {
        uart_write_string(cmd_list[i].name);
        uart_write_string("\t\t: ");
        uart_write_string(cmd_list[i].help);
    }
}
// static void hello_cmd(int argc, char *argv[])

struct shell_cmd cmd_list[] = {
    {.name="ls", .help="list files in initramfs\n", .func=ls_cmd},
    {.name="cat", .help="print content of a specific file in initramfs\n", .func=cat_cmd},
    {.name="dtb", .help="print device tree properties\n", .func=dtb_cmd},
    {.name="help", .help="print this help menu\n", .func=help_cmd},
    {.name="fork", .help="test_fork\n", .func=fork_cmd},
    {.name="kill", .help="kill shell\n", .func=kill_cmd},
    {.name="hello", .help="print Hello World!\n", .func=hello_cmd},
    {.name="async", .help="read and write a line asynchronously\n", .func=async_cmd},
    {.name="buddy", .help="test buddy system\n", .func=buddy_cmd},
    {.name="malloc", .help="test dynamic memory allocation\n", .func=malloc_cmd},
    {.name="thread", .help="test kernel thread round-robin scheduling\n", .func=thread_cmd},
    {.name="reboot", .help="reboot the device\n", .func=reboot_cmd},
    {.name="mknote", .help="make a note\n", .func=mknote_cmd},
    {.name="linote", .help="list all notes\n", .func=linote_cmd},
    {.name="ldprog", .help="run program in EL0 without interrupt\n", .func=ldprog_cmd},
    {.name="uptime", .help="write system uptime\n", .func=uptime_cmd},
    {.name="timeout", .help="write message at the given time\n", .func=timeout_cmd},
    {.name="preempt", .help="schedule a timer function which asynchronously write 'asyncw'.\n", .func=preempt_cmd},
    
};

size_t shell_cmd_cnt = ARRAY_SIZE(cmd_list);

enum ANSI_ESC
{
    Unknown,
    CursorForward,
    CursorBackward,
    Delete
};

enum ANSI_ESC decode_csi_key()
{
    char c = kuart_read();
    switch (c)
    {
    case 'C':
        return CursorForward;
        break;
    case 'D':
        return CursorBackward;
        break;
    case '3':
        return kuart_read() == '~' ? Delete : Unknown;
        break;
    default:
        return Unknown;
        break;
    }
}

enum ANSI_ESC decode_ansi_escape()
{
    return kuart_read() == '[' ? decode_csi_key() : Unknown;
}

unsigned int shell_read_string(char *cmd, unsigned int cmd_size)
{
    uart_write_string("# ");

    int idx = 0, end = 0, i, last_end = 0;
    cmd[0] = '\0';
    char c;
    while ((c = kuart_read()) != '\n')
    {
        // Decode CSI key sequences
        if (c == 27) {
            enum ANSI_ESC key = decode_ansi_escape();
            switch (key)
            {
            case CursorForward:
                if (idx < end)
                    idx++;
                break;

            case CursorBackward:
                if (idx > 0)
                    idx--;
                break;

            case Delete:
                // left shift command
                for (i = idx; i < end; i++)
                {
                    cmd[i] = cmd[i + 1];
                }
                cmd[--end] = '\0';
                break;

            case Unknown:
                uart_flush();
                break;
            }
        }
        // CTRL-C
        else if(c == 3) {
            cmd[0] = '\0';
            goto shell_input_loop_end;
        }
        else if (c == 8 || c == 127) {
        // Backspace
            if (idx > 0)
            {
                idx--;
                // left shift command
                for (i = idx; i < end; i++)
                {
                    cmd[i] = cmd[i + 1];
                }
                cmd[--end] = '\0';
            }
        }
        // Insert new character to cmd
        else {
            if (end == cmd_size)
                return end;
            // right shift command
            if (idx < end)
            {
                for (i = end; i > idx; i--)
                {
                    cmd[i] = cmd[i - 1];
                }
            }
            cmd[idx++] = c;
            cmd[++end] = '\0';
        }

        uart_write_retrace();
        uart_write_string("# ");
        uart_write_string(cmd);
        for (int j = 0; j < (last_end - end); j++)
            kuart_write(' ');
        uart_write_retrace();
        uart_write_string("\e[");
        uart_write_no((unsigned int)(idx + 2));
        uart_write_string("C");
        last_end = end;
    }
shell_input_loop_end:
    uart_write_string("\n");
    return (unsigned int)end;
}

int run_if_builtin(int argc, char *argv[])
{
    for (size_t i = 0; i < shell_cmd_cnt; i++) {
        if (strcmp(argv[0], cmd_list[i].name) == 0) {
            cmd_list[i].func(argc, argv);
            return 1;
        }
    }
    return 0;
}

void shell_process_cmd(char *input_buffer, unsigned int input_size)
{
    /* split first arg and others*/
    char *argv[MAX_ARGS+1];
    int argc = shell_parse_argv(input_buffer, argv, MAX_ARGS);
    if (!run_if_builtin(argc, argv))
    {
        uart_write_string("Command not found!\n");
    }
}

int shell_parse_argv(char *s, char *argv[], unsigned max_args)
{
    char *token = strtok(s, " \n");
    int argc = 0;
    while (token != NULL && argc < max_args) {
        argv[argc++] = token;
        token = strtok(NULL, " \n");
    }
    argv[argc] = NULL;
    return argc;
}

void shell_main(void)
{
    /* shell */
    unsigned int read_cnt;
    char input_buffer[MAX_SHELL_INPUT];

    while (1) {
        read_cnt = shell_read_string(input_buffer, MAX_SHELL_INPUT);
        shell_process_cmd(input_buffer, read_cnt);
    }

}

void shell_main_thread(void)
{
    size_t read_cnt;
    char input_buffer[MAX_SHELL_INPUT];
    while (1) {
        if (UART_READABLE()) {
            read_cnt = shell_read_string(input_buffer, MAX_SHELL_INPUT);
            shell_process_cmd(input_buffer, read_cnt);
        }
        schedule();
    }
}