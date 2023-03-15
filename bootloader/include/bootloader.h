#define KERNEL_BASE ((volatile char *)(0x80000))

void load_kernel();
unsigned int get_kernel_size();
