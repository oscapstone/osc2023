#include "fs/file.h"

int read(struct file * file, char * buf, unsigned int n) {
    if (!file->read) {
        return FILE_EOPNULL;
    }
    return file->read(file->_f, buf, n);
}

int write(struct file * file, char * buf, unsigned int n) {
    if (!file->write) {
        return FILE_EOPNULL;
    }
    return file->write(file->_f, buf, n);
}
