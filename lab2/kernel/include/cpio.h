#ifndef _CPIO_H
#define _CPIO_H

void initrd_list();
void cat_list ();
void callback_initramfs(void * addr, int size);
int get_initramfs();

#endif