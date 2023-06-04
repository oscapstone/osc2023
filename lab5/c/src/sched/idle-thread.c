#include "oscos/sched.h"

void idle(void) {
  for (;;) {
    kill_zombies();
    schedule();
  }
}
