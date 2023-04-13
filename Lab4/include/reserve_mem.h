typedef struct _reserved_memory_block
{
    unsigned long start;
    unsigned long end;
    char name[30];
} reserved_memory_block;

void memory_reserve(unsigned long start, unsigned long end, char *name);
int check_contain_RM(unsigned long start, unsigned long end);
void memory_init();