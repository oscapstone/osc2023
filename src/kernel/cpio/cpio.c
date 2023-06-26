#include "cpio/cpio.h"
#include "utils.h"
#include "peripherals/mini_uart.h"
#include "process.h"
#include "fs/tmpfs.h"
#include "fs/vfs.h"
#include "mmu/mmu-def.h"

// extern void thread_exec_wrapper;
char *initramfs_addr;

void set_initramfs_addr(uint32_t addr) {
    initramfs_addr = (char*)kernel_pa2va(addr);
}

unsigned int hex_to_uint(char* c) {
    unsigned int ret = 0;
    for(int i = 0; i < 8; i ++, c ++) {
        if(*c <= 'f' && *c >= 'a') {
            ret = (ret << 4) + *c - 'a' + 10;
        } else if(*c <= 'F' && *c >= 'A') {
            ret = (ret << 4) + *c - 'A' + 10;
        } else if(*c <= '9' && *c >= '0') {
            ret = (ret << 4) + *c - '0';
        } else {
            return 0;
        }
    }
    return ret;
}

void list_files() {
    char *tmp = (char *)initramfs_addr;
    int offset = sizeof(cpio_newc_header);
    for(;;) {
        unsigned int namesize = hex_to_uint(((cpio_newc_header*)tmp)->c_namesize);
        unsigned int filesize = hex_to_uint(((cpio_newc_header*)tmp)->c_filesize);
        unsigned int filemode = hex_to_uint(((cpio_newc_header*)tmp)->c_mode);

        if(strncmp(tmp + offset, "TRAILER!!!", 10) == 0) {
            break;
        }
        tmp += (((((namesize + offset) & 0x3) ^ 0x3) + 1) & 0x3) + namesize + offset;
        tmp += ((((filesize & 0x3) ^ 0x3) + 1) & 0x3) + filesize;
    }
}
unsigned int cat_file(const char *filename) {
    char *tmp = (char *)initramfs_addr;
    int offset = sizeof(cpio_newc_header);
    for(;;) {
        unsigned int namesize = hex_to_uint(((cpio_newc_header*)tmp)->c_namesize);
        unsigned int filesize = hex_to_uint(((cpio_newc_header*)tmp)->c_filesize);


        if(strncmp(tmp + offset, "TRAILER!!!", 10) == 0) {
            uart_send_string("file not found!\r\n");
            return 255;
        }
        if(strncmp(tmp + offset, filename, namesize) == 0) {
            uart_send_n(tmp + (((((namesize + offset) & 0x3) ^ 0x3) + 1) & 0x3) + namesize + offset, filesize);
            return 0;
        }
        // Since the (name + offset) size will be pad to align 4bytes
        tmp += (((((namesize + offset) & 0x3) ^ 0x3) + 1) & 0x3) + namesize + offset;
        // Since the filesize will be pad to align 4 bytes
        tmp += ((((filesize & 0x3) ^ 0x3) + 1) & 0x3) + filesize;
    }
}

unsigned int load_program(const char *filename) {
    process_exec(filename, NULL, 0);
}

uint8_t get_file(const char *filename, char **content, unsigned int *c_filesize) {
    uart_send_string("in get file\r\n");
    char *tmp = (char *)kernel_pa2va(initramfs_addr);
    int offset = sizeof(cpio_newc_header);
    for(;;) {
        unsigned int namesize = hex_to_uint(((cpio_newc_header*)tmp)->c_namesize);
        unsigned int filesize = hex_to_uint(((cpio_newc_header*)tmp)->c_filesize);

        if(strncmp(tmp + offset, "TRAILER!!!", 10) == 0) {
            uart_send_string("file not found!\r\n");
            return 255;
        }
        if(strncmp(tmp + offset, filename, namesize) == 0) {
            uint64_t content_offset = (((((namesize + offset) & 0x3) ^ 0x3) + 1) & 0x3) + namesize + offset;
            *content = (char *)(content_offset + tmp);
            *c_filesize = filesize;
            return 0;
        }
        // Since the (name + offset) size will be pad to align 4bytes
        tmp += (((((namesize + offset) & 0x3) ^ 0x3) + 1) & 0x3) + namesize + offset;
        // Since the filesize will be pad to align 4 bytes
        tmp += ((((filesize & 0x3) ^ 0x3) + 1) & 0x3) + filesize;
    }
}



struct filesystem cpio_fs = {
    .name = "cpio_fs",
    .root = NULL,
    // .init_fs = &init_tmpfs,
    .init_fs = &init_cpio_fs,
    .setup_mount = &mount_tmpfs
};

struct vnode_operations cpio_vops = {
    .create = &vops_not_support,
    .lookup = &tmpfs_lookup,
    .mkdir = &vops_not_support
};

struct file_operations cpio_fops = {
    .close = &tmpfs_close,
    .ioctl = &fops_not_support,
    .lseek64 = &tmpfs_lseek64,
    .open = &tmpfs_open,
    .read = &tmpfs_read,
    .stat = &tmpfs_stat,
    .write = &fops_not_support
};

int init_cpio_fs(struct filesystem *fs) {
    init_tmpfs(fs);
    return 0;
}

void parse_cpio() {
    struct vnode* cpio_root = cpio_fs.root;
    cpio_root->flag = VFS_DIR;

    char *tmp = (char *)initramfs_addr;
    int offset = sizeof(cpio_newc_header);
    char name[100];
    for(;;) {
        unsigned int namesize = hex_to_uint(((cpio_newc_header*)tmp)->c_namesize);
        unsigned int filesize = hex_to_uint(((cpio_newc_header*)tmp)->c_filesize);
        unsigned int filemode = hex_to_uint(((cpio_newc_header*)tmp)->c_mode) & C_FMASK;

        if(strncmp(tmp + offset, "TRAILER!!!", 10) == 0) {
            break;
        }

        if(strncmp(tmp + offset, ".", 1) != 0) {
            if(filemode == C_REG) {
                struct vnode *newnode;
                memcpy(name, "/initramfs/", 11);
                memcpy(name + 11, tmp + offset, namesize);
                name[12 + namesize] = '\0';
                vfs_create(name, 0777, &newnode);
                struct tmpfs_vinfo* info = newnode->internal;
                uint64_t content_offset = (((((namesize + offset) & 0x3) ^ 0x3) + 1) & 0x3) + namesize + offset;
                info->content = (char *)(content_offset + tmp);
                info->filesize = filesize;
            } else if(filemode == C_DIR) {
                strncpy(name, "/initramfs/", 12);
                strncpy(name + 12, tmp + offset, namesize);
                name[12 + namesize] = '\0';
                vfs_mkdir(name);
            }
        } 
        tmp += (((((namesize + offset) & 0x3) ^ 0x3) + 1) & 0x3) + namesize + offset;
        tmp += ((((filesize & 0x3) ^ 0x3) + 1) & 0x3) + filesize;
    }
    cpio_fs.root->f_ops = &cpio_fops;
    cpio_fs.root->v_ops = &cpio_vops;

}


void mount_initramfs() {
    vfs_mkdir("/initramfs");
    register_filesystem(&cpio_fs);
    vfs_mount("/initramfs", "cpio_fs");

    parse_cpio();

    return;
}