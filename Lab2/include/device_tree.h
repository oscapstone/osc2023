#ifndef _DEVICE_TREE_H
#define _DEVICE_TREE_H

typedef int (*fdt_callback)(void *);
int initramfs_callback(void *dtb);
int dtb_parser(void *dtb);
void fdt_traverse(fdt_callback cb, char *dtb);

#endif /*_DEVICE_TREE_H */