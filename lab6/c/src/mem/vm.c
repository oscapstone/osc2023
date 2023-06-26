#include "oscos/mem/vm.h"

#include <stdint.h>

// Symbol defined in the linker script.
extern char _kernel_vm_base[];

pa_t kernel_va_to_pa(const void *const va) {
  return (pa_t)((uintptr_t)va - (uintptr_t)_kernel_vm_base);
}

void *pa_to_kernel_va(const pa_t pa) {
  return (void *)((uintptr_t)pa + (uintptr_t)_kernel_vm_base);
}

pa_range_t kernel_va_range_to_pa_range(const va_range_t range) {
  return (pa_range_t){.start = kernel_va_to_pa(range.start),
                      .end = kernel_va_to_pa(range.end)};
}
