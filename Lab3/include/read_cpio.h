#ifndef _READ_CPIO_H
#define _READ_CPIO_H

void read_cpio(char *cpioDest);
void read_content(char *cpioDest, char *filename);
char *find_content_addr(char *cpioDest, char *filename);
int load_userprogram(char *cpioDest, char *userDest);

#endif /*_READ_CPIO_H */