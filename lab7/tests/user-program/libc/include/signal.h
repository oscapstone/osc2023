#include "unistd.h"

#include "oscos-uapi/signal.h"

int kill(pid_t pid);

sighandler_t signal(int signal, sighandler_t handler);

int signal_kill(pid_t pid, int signal);
