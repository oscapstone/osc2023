int get_board_revision(unsigned int* board_revision);
int get_arm_memory_info(unsigned int* base_addr,unsigned int* size);
void set(long addr, unsigned int value);
void reboot();
void reset(int tick);
void cancel_reset();