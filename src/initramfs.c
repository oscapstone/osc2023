#include "initramfs.h"
#include "uart.h"
#include "utils.h"
#include "thread.h"
#include "mm.h"
#include "syscall.h"

char *cpio_addr, *cpio_end;
struct initramfs _initramfs;
extern void run_user_program(void *prog_addr, void *stack_addr);

void _cpio_ls(struct initramfs *self, char *path) 
{
    char *ptr = self->addr;

    if (strncmp(ptr, "070701", 6) != 0) {
        uart_write_string("Not a valid New ASCII Format Cpio archive file\n");
        return;
    }
    // ptr += 6;
    unsigned file_size, path_size, header_path_size;
    char *file_path;
    struct cpio_newc_header *header;
    while (1) {
        header = ptr;
        
        file_size = hex2unsigned(&(header->c_filesize));
        file_path = ptr + sizeof(struct cpio_newc_header);

        if (strncmp(file_path, "TRAILER!!!", 10) == 0) {
            //end of archive
            break;
        }

        path_size = hex2unsigned(&(header->c_namesize));
        header_path_size = sizeof(struct cpio_newc_header) + path_size;
        if (strstartswith(file_path, path)) {
            uart_write_string(file_path);
            uart_write_string("\n");
        }
        // skip the file content
        /*
         This expression adds 3 to the size of an integer to round 
         it up to the nearest multiple of 4 bytes,
         then applies the mask ~3 to set the last two bits to 0.
        */
        ptr += ((file_size + 3) & ~3) + ((header_path_size + 3) & ~3);
    }
}

struct cpio_newc_header *_cpio_find_file(struct initramfs *self, char *path)
{
    char *ptr = self->addr;
    if (strncmp(ptr, "070701", 6) != 0) {
        uart_write_string("Not a valid New ASCII Format Cpio archive file\n");
        return;
    }
    unsigned file_size, path_size, header_path_size;
    char *file_path;
    struct cpio_newc_header *header;
    while (1) {
        header = ptr;
        
        file_path = ptr + sizeof(struct cpio_newc_header);
        uart_write_string(file_path);
        uart_write_string("\n");
        if (strcmp(file_path, path) == 0) {
            return header;
        }

        if (strncmp(file_path, "TRAILER!!!", 10) == 0) {
            //end of archive
            break;
        }

        file_size = hex2unsigned(&(header->c_filesize));
        path_size = hex2unsigned(&(header->c_namesize));
        header_path_size = sizeof(struct cpio_newc_header) + path_size;
        // skip the file content
        ptr += ALIGN(file_size, 4) + ALIGN(header_path_size, 4);
    }
    return NULL;
}

char *_cpio_file_content(struct initramfs *self, char *path, size_t *fsize)
{
    char *ptr = self->addr;

    if (strncmp(ptr, "070701", 6) != 0) {
        uart_write_string("Not a valid New ASCII Format Cpio archive file\n");
        return NULL;
    }

    struct cpio_newc_header *header = _cpio_find_file(self, path);
    if (header == NULL) {
        uart_write_string("File Not Found!\n");
        return NULL;
    }
    size_t file_size, path_size, header_path_size;
    file_size = hex2unsigned(&(header->c_filesize));
    path_size = hex2unsigned(&(header->c_namesize));
    header_path_size = sizeof(struct cpio_newc_header) + path_size;
    header_path_size = ALIGN(header_path_size, 4);//((header_path_size + 3) & ~3);
    char *content_ptr = (char *)header + header_path_size;
    *fsize = file_size;
    return content_ptr;
}

void _cpio_cat(struct initramfs *self, char *path) 
{
    size_t file_size;
    char *content_ptr = self->file_content(self, path, &file_size);
    for (int i = 0; i < file_size; i++, content_ptr++) {
        kuart_write(*content_ptr);
    }
    // uart_write_string("\n");
}

int _cpio_exec(struct initramfs *self, char *argv[])
{
    // char *ptr = self->addr;

    // char *path = argv[0];

    // unsigned file_size, path_size, header_path_size;

    // // char *content_ptr = self->file_content(self, path, &file_size);
    // ///////////////////////////////////////////////////

    // if (strncmp(ptr, "070701", 6) != 0) {
    //     uart_write_string("Not a valid New ASCII Format Cpio archive file\n");
    //     return -1;
    // }

    // struct cpio_newc_header *header = _cpio_find_file(self, path);
    // if (header == NULL) {
    //     uart_write_string("File Not Found!\n");
    //     return -1;
    // }
    // file_size = hex2unsigned(&(header->c_filesize));
    // path_size = hex2unsigned(&(header->c_namesize));
    // header_path_size = sizeof(struct cpio_newc_header) + path_size;
    // header_path_size = ALIGN(header_path_size, 4);
    // char *content_ptr = (char *)header + header_path_size;

    char *path = argv[0];
    unsigned file_size;
    struct vnode *file_node;
    if (vfs_lookup(path, &file_node)) {
        return NULL;
    }
    char *content_ptr = (char *)(file_node->internal);
    struct dentry *dent;
    char *file_name = path + strlen(path)-1;
    while (*(file_name-1) != '/') file_name--;
    example_lookup_d(file_node->parent, &dent, file_name);
    file_size = *(dent->file_size);

    /////////////////////////////////////////////////////
    // char *load_addr = load_program(content_ptr, file_size);
    /////////////////////////////////////////////////////
    task_t *current = get_current_thread();
    if (current->user_text != NULL) {
        free_page(current->user_text);
        current->user_text = NULL;
    }
    size_t page_needed = ALIGN(file_size, PAGE_SIZE) / PAGE_SIZE;
    char *load_addr = alloc_pages(page_needed);
    if (load_addr == NULL) {
        uart_write_string("No enough space for loading program.\n");
        return NULL;
    }
    memcpy(load_addr, content_ptr, file_size);
    current->user_text = load_addr;
    mappages(current->pgd, 0x0, file_size, VA2PA(load_addr));

    //////////////////////////////////////////////////////
    // run_user_prog(load_addr);
    //////////////////////////////////////////////////////
    mappages(current->pgd, USER_STK_LOW, STACKSIZE, 0);
    update_pgd(VA2PA(current->pgd));
    //write lr
    __asm__ __volatile__("mov x30, %[value]"
                         :
                         : [value] "r" (exit)
                         : "x30");
    //write fp
    __asm__ __volatile__("mov x29, %[value]"
                         :
                         : [value] "r" (USER_STK_HIGH)
                         : "x29");
    //allow interrupt
    write_sysreg(spsr_el1, 0);
    write_sysreg(elr_el1, 0x0);
    write_sysreg(sp_el0, USER_STK_HIGH);
    asm volatile("eret");
    return -1;
}

void init_initramfs(struct initramfs *fs) 
{
    fs->addr = cpio_addr;
    fs->ls = _cpio_ls;
    fs->cat = _cpio_cat;
    fs->file_content = _cpio_file_content;
    fs->exec = _cpio_exec;

    if (strncmp(cpio_addr, "070701", 6) != 0) {
        uart_write_string("Not a valid New ASCII Format Cpio archive file\n");
        return;
    }
}

//////////////////////////////// functions as interfaces for vfs /////////////////////////////////////
struct file_operations initramfs_f_ops = {
    .write = initramfs_write,
    .read = initramfs_read,
    .open = initramfs_open,
    .close = initramfs_close,
    .lseek64 = initramfs_lseek64
};

struct vnode_operations initramfs_v_ops = {
    .lookup = initramfs_lookup,
    .create = initramfs_create,
    .mkdir = initramfs_mkdir
};



static int initramfs_init_file(struct vnode *dir_node, struct vnode **target, char *path, char *ptr, size_t file_size)
{
    int errno;
    //Expect path do not contain any directory
    struct vnode *dummyv;
    if ((errno = initramfs_lookup(dir_node, &dummyv, path)) == 0) {
        return 1; //file exists
    }
    (*target)->internal = ptr;
    size_t *fsize = (size_t *)kmalloc(sizeof(size_t));
    *fsize = file_size;
    struct dentry *dummy;
    if ((errno = example_append2dir(dir_node, *target, fsize, path, NULL, &dummy))) {
        return errno;
    }
    
    return 0;
    //TODO: nested init
}

int initramfs_setup_mount(struct filesystem* fs, struct mount* mount)
{
    int errno;
    //dfs to create vfs nodes
    char *ptr = cpio_addr;
    if (strncmp(ptr, "070701", 6) != 0) {
        uart_write_string("Not a valid New ASCII Format Cpio archive file\n");
        return 1;
    }

    struct vnode *root_vnode = mount->root;
    INIT_LIST_HEAD((list_t *)(&(root_vnode->internal)));
    root_vnode->v_ops = &initramfs_v_ops;
    root_vnode->f_ops = &initramfs_f_ops;

    unsigned file_size, path_size, header_path_size;
    char *file_path;
    struct cpio_newc_header *header;
    while (1) {
        header = ptr;
        
        file_path = ptr + sizeof(struct cpio_newc_header);
        // uart_write_string(file_path);
        // if (strcmp(file_path, path) == 0) {
        //     return header;
        // }

        if (strncmp(file_path, "TRAILER!!!", 10) == 0) {
            //end of archive
            break;
        }

        file_size = hex2unsigned(&(header->c_filesize));
        path_size = hex2unsigned(&(header->c_namesize));
        header_path_size = sizeof(struct cpio_newc_header) + path_size;
        header_path_size = ALIGN(header_path_size, 4);
        char *content_ptr = (char *)header + header_path_size;

        struct vnode *target = (struct vnode *)kmalloc(sizeof(struct vnode));
        if (target == NULL) return 1;
        target->v_ops = root_vnode->v_ops;
        target->f_ops = root_vnode->f_ops;
        target->file_type = REGULAR_FILE;
        target->mount = NULL;
        target->parent = root_vnode;
        if (errno = initramfs_init_file(root_vnode, &target, file_path, content_ptr, file_size)) {
            return errno;
        }

        // skip the file content
        ptr += ALIGN(file_size, 4) + ALIGN(header_path_size, 4);
    }
    return 0;
}

int initramfs_write(struct file* file, const void* buf, size_t len)
{
    return 0;
}

int initramfs_read(struct file* file, void* buf, size_t len)
{
    char *start = (char *)(file->vnode->internal);
    memcpy(buf, start + file->f_pos, len);
    file->f_pos += len;
    return len;
}

int initramfs_open(struct vnode* file_node, struct file** target)
{
    struct file *f = *target;
    f->f_ops = &initramfs_f_ops;
    f->f_pos = 0;
    //READ ONLY
    f->flags = FILE_READ;
    return 0;
}

int initramfs_close(struct file* file)
{
    memset(file, 0, sizeof(struct file));
    return 0;
}

long initramfs_lseek64(struct file* file, long offset, int whence)
{
    //Expect new offset will not out of file range
    if (whence == SEEK_CUR)
        file->f_pos += offset;
    else if (whence == SEEK_SET)
        file->f_pos = offset;
    return file->f_pos;
}



int initramfs_lookup(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
    //Expect path do not contain any directory
    return example_lookup(dir_node, target, component_name);
    //TODO: nested directory
}

int initramfs_create(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
    //read only
    return 1;
}

int initramfs_mkdir(struct vnode* dir_node, struct vnode** target, const char* component_name)
{
    //read only
    return 1;
}