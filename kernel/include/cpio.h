#define SIZE_OF_MAGIC 6
#define SIZE_OF_FEILD 8
#define PADDING_MULTIPLE 4

#define CPIO_MAGIC_NUMBER_STRING "070701"
#define END_IDENTIFIER "TRAILER!!!"
#define NOW_DIRECTORY "."
#define PREVIOUS_DIRECTORY ".."

static char *cpio_base;

void init_cpio(char *address);
void list_files();
void print_file(char *command);