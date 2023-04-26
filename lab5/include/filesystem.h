#ifndef FILESYSTEM_H
#define FILESYSTEM_H

extern void* CPIO_DEFAULT_PLACE;

int  ls(char* working_dir);
int  cat(char* thefilepath);
int  execfile(char* thefilepath);
unsigned int get_file_size(char *thefilepath);
char *get_file_start(char *thefilepath);

#endif