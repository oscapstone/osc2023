#ifndef INIT_FDT
#define INIT_FDT

struct fdt_node;
struct fdt_prop;

struct fdt {
    char * addr;
    char * dt_struct;
    char * dt_string;
    struct fdt_node * root;
};

struct fdt_node {
    char * name;
    unsigned int level;
    struct fdt_node * chdrn;
    struct fdt_node * slbng;
    struct fdt_prop * prpty;
};

struct fdt_prop {
    struct fdt_prop * next;
    char * name;
    unsigned int len;
    void * val;
};

void fdt_init(char * addr);
void fdt_traverse(void (* callback)(struct fdt_node * node),
                  unsigned int (* cmp)(struct fdt_node * node));

#endif