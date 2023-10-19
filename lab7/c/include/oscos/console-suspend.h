#ifndef OSCOS_CONSOLE_SUSPEND_H
#define OSCOS_CONSOLE_SUSPEND_H

#include <stddef.h>

#include "oscos/uapi/unistd.h"

ssize_t console_write_suspend(const char *buf, size_t size);
ssize_t console_read_suspend(char *buf, size_t size);

#endif
