#ifndef __KERNEL__
#define __KERNEL__
#define BUF_SIZE 256
void _init();
void boot_message();
void allocator_test();
void devicetree_check();
void spsr_el1_check();
void boottimer();
void allocator_test2();
#endif
