#ifndef FS_FILE
#define FS_FILE

#define FILE_CPIO       0x01U

#define FILE_EOPNULL    ~(0x0)

struct file {
    void * _f;
    int (* read)(void * _f, char * buf, unsigned int n);
    int (* write)(void * _f, char * buf, unsigned int n);
};

int read(struct file * file, char * buf, unsigned int n);
int write(struct file * file, char * buf, unsigned int n);

#endif