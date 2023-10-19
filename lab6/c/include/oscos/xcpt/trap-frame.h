#ifndef OSCOS_XCPT_TRAP_FRAME_H
#define OSCOS_XCPT_TRAP_FRAME_H

#include <stdalign.h>
#include <stdint.h>

typedef struct {
  alignas(16) union {
    struct {
      uint64_t r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14,
          r15, r16, r17, r18, lr;
    };
    uint64_t regs[20];
  };
} trap_frame_t;

typedef struct {
  alignas(16) struct {
    uint64_t spsr, elr;
  };
  trap_frame_t trap_frame;
} extended_trap_frame_t;

#endif
