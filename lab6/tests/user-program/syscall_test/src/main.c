#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"

#include "unistd.h"

#define NS_PER_SEC 1000000000

void delay_ns(const uint64_t ns) {
  uint64_t start_counter_val;
  __asm__ __volatile__("mrs %0, cntpct_el0" : "=r"(start_counter_val)::);

  uint64_t counter_clock_frequency_hz;
  __asm__ __volatile__("mrs %0, cntfrq_el0"
                       : "=r"(counter_clock_frequency_hz)::);
  counter_clock_frequency_hz &= 0xffffffff;

  // ceil(ns * counter_clock_frequency_hz / NS_PER_SEC).
  const uint64_t delta_counter_val =
      (ns * counter_clock_frequency_hz + (NS_PER_SEC - 1)) / NS_PER_SEC;

  for (;;) {
    uint64_t counter_val;
    __asm__ __volatile__("mrs %0, cntpct_el0" : "=r"(counter_val)::);

    if (counter_val - start_counter_val >= delta_counter_val)
      break;
  }
}

void fork_test(void) {
  printf("\nFork Test, pid %d\n", getpid());
  int cnt = 1;
  int ret = 0;
  if ((ret = fork()) == 0) { // child
    long long cur_sp;
    __asm__ __volatile__("mov %0, sp" : "=r"(cur_sp));
    printf("first child pid: %d, cnt: %d, ptr: %p, sp : %llx\n", getpid(), cnt,
           (void *)&cnt, cur_sp);
    ++cnt;

    if ((ret = fork()) != 0) {
      __asm__ __volatile__("mov %0, sp" : "=r"(cur_sp));
      printf("first child pid: %d, cnt: %d, ptr: %p, sp : %llx\n", getpid(),
             cnt, (void *)&cnt, cur_sp);
    } else {
      while (cnt < 5) {
        __asm__ __volatile__("mov %0, sp" : "=r"(cur_sp));
        printf("second child pid: %d, cnt: %d, ptr: %p, sp : %llx\n", getpid(),
               cnt, (void *)&cnt, cur_sp);
        delay_ns(1000000);
        ++cnt;
      }
    }
    exit(0);
  } else {
    printf("parent here, pid %d, child %d\n", getpid(), ret);
  }
}

void main(void) { fork_test(); }
