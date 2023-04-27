#ifndef __LOCK_H
#define __LOCK_H
#include "type.h"
struct k_lock {
    uint64_t lock;
};

struct k_lock* k_lock_new();
void k_lock_get(struct k_lock *lock);
void k_lock_release(struct k_lock *lock);
#endif