#ifndef SYSTEM_H
#define SYSTEM_H

extern char* _dtb;

int get_board_revision(unsigned int* board_revision);
int get_arm_memory_info(unsigned int* base_addr,unsigned int* size);
void set(long addr, unsigned int value);
void reboot();
void reset(int tick);
void cancel_reset();
void load_kernel();

#endif