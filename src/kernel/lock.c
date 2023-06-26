#include "lock.h"
#include "mem/mem.h"
#include "peripherals/mini_uart.h"
#include "interrupt.h"

struct k_lock* k_lock_new() {
    struct k_lock* ret = (struct k_lock*)simple_malloc(sizeof(struct k_lock));
    ret->lock = 0;
    return ret;
}

void k_lock_get(struct k_lock *lock) {
    uint64_t flag = interrupt_disable_save();
    while(lock->lock){
        enable_interrupt();
        // asm volatile("nop");
    }
    disable_interrupt();
    lock->lock = 1;
    interrupt_enable_restore(flag);
}
void k_lock_release(struct k_lock *lock) {
    uint64_t flag = interrupt_disable_save();
    lock->lock = 0;
    interrupt_enable_restore(flag);
}