#ifndef _LOAD_KERNEL_H
#define _LOAD_KERNEL_H

void load_kernel(char *dest);
void relocate(char *from_dest, char *to_dest);

#endif /*_LOAD_KERNEL_H */
