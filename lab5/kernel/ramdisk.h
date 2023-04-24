#ifndef	RAMDISK_H
#define	RAMDISK_H

void init_ramdisk(void);
void ramdisk_ls(void);
void ramdisk_cat(void);
char* ramdisk_find_file(char* filename);

#endif /* RAMDISK_H */