#ifndef INIT_RAMFS
#define INIT_RAMFS

#define RAMFS_ADDR  ((char *)0x8000000U)

void ramfs_ls();
void ramfs_cat(char * filename);

#endif