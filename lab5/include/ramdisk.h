#ifndef RAMDISK_H

#define RAMDISK_H

void init_rd(char *buffer);
int oct_to_int(char *str,int n);
int bufcmp(void *str1,void *str2,int n);
void ls();
void cat();
char* find_prog(char* buffer,char* target);
int find_prog_size(char* buffer,char* target);

#endif
