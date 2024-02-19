#ifndef _READ_CPIO_H
#define _READ_CPIO_H

typedef struct cpio_node_struct
{
    char *name;
    int type;
    int size;
    char *data;
    struct cpio_node_struct *next;
} cpio_node_t;

void read_cpio(char *cpioDest);
void read_content(char *cpioDest, char *filename);
char *find_content_addr(char *cpioDest, const char *filename);
int load_userprogram(const char *, char *);

#endif /*_READ_CPIO_H */