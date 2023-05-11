#include "oscos/timer/delay.h"

#include <stdbool.h>

#include "oscos/timer/timeout.h"
#include "oscos/utils/suspend.h"

static void _timeout_callback(volatile bool *const flag) { *flag = true; }

void delay_ns(const uint64_t ns) {
  volatile bool flag = false;
  timeout_add_timer((void (*)(void *))_timeout_callback, (void *)&flag, ns);

  WFI_WHILE(!flag);
}
