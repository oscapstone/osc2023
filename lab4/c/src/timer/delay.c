#include "oscos/timer/delay.h"

#include <stdbool.h>

#include "oscos/timer/timeout.h"

static void _timeout_callback(bool *const volatile flag) { *flag = true; }

void delay_ns(const uint64_t ns) {
  volatile bool flag = false;
  timeout_add_timer((void (*)(void *))_timeout_callback, (void *)&flag, ns);

  while (!flag) {
    __asm__ __volatile__("wfi");
  }
}
