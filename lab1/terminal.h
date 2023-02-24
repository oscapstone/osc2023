#ifndef TERMINAL_H
#define TERMINAL_H

#define PM_PASSWORD 	0x5a000000
#define PM_RSTC		(unsigned int*)0x3f10001c
#define PM_WDOG		(unsigned int*)0x3f100024

void terminal_run(void);
int help(void);
int reboot(void);
int hello(void);
int invalid_command(const char*);
int lshw(void);

#endif //TERNINAL_H
