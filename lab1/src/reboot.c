#define PM_PASSWORD   0x5a000000
#define PM_RSTC       0x3F10001c
#define PM_WDOG       0x3F100024

void set(long addr, unsigned int value) {
  volatile unsigned int* point = (unsigned int *)addr;
  *point = value;
}

void reset(int tick) {
  set(PM_RSTC, PM_PASSWORD | 0x20);
  set(PM_WDOG, PM_PASSWORD | tick);
}

void cancel_reset() {
  set(PM_RSTC, PM_PASSWORD | 0);
  set(PM_WDOG, PM_PASSWORD | 0);
}
