#ifndef	RAMDISK_H
#define	RAMDISK_H

void ramdisk_ls(void);
void ramdisk_cat(void);
int ramdisk_load_file_to_adr(char* adr);

#endif /* RAMDISK_H */