#ifndef OSCOS_RESET_H
#define OSCOS_RESET_H

#include <stdint.h>

void reset(uint32_t tick);
void cancel_reset(void);

#endif
