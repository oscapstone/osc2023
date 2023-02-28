#define SHELL_PROMPT "# "
#define HELP "help"
#define HELLO "hello"
#define REBOOT "reboot"
#define REVISION "revision"
#define MEMORY "memory"

#define PM_PASSWORD 0x5a000000
#define PM_RSTC 0x3F10001c
#define PM_WDOG 0x3F100024

void start_shell();