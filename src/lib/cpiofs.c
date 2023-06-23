#include <cpiofs.h>
#include <cpio.h>
#include <string.h>
#include <mm.h>
#include <dt17.h>
#include <panic.h>
#include <preempt.h>

static struct vnode cpio_root_node;
static struct vnode mount_old_node;
static int cpio_mounted;

static struct filesystem cpiofs = {
    .name = "cpiofs",
    .mount = cpiofs_mount
};

static struct vnode_operations cpiofs_v_ops = {
    .lookup = cpiofs_lookup,
    .create = cpiofs_create,
    .mkdir = cpiofs_mkdir,
    .isdir = cpiofs_isdir,
    .getname = cpiofs_getname,
    .getsize = cpiofs_getsize
};

static struct file_operations cpiofs_f_ops = {
    .write = cpiofs_write,
    .read = cpiofs_read,
    .open = cpiofs_open,
    .close = cpiofs_close,
    .lseek64 = cpiofs_lseek64,
    .ioctl = cpiofs_ioctl
};

static struct vnode *get_dir_vnode(struct vnode *dir_node, const char **pathname){
    struct vnode *result;
    const char *start;
    const char *end;
    char buf[0x100];

    start = end = *pathname;

    if(*start == '/')
        result = &cpio_root_node;
    else
        result = dir_node;

    while(1){
        if(!strncmp("./", start, 2)){
            start += 2;
            end = start;
            continue;
        }
        else if(!strncmp("../", start, 3)){
            if(result->parent){
                result = result->parent;
            }

            start += 3;
            end = start;
            continue;
        }
        while(*end != '\0' && *end != '/')
            end++;
        if(*end == '/'){
            int ret;
            if(start == end){
                end++;
                start = end;
                continue;
            }

            // TODO: Check if the length is less than 0x100
            memncpy(buf, start, end - start);
            buf[end - start] = 0;
            ret = result->v_ops->lookup(result, &result, buf);
            if(ret < 0)
                return NULL;
            end++;
            start = end;
        }
        else{
            break;
        }
    }
    *pathname = *start ? start : NULL;
    return result;
}

static void cpio_init_mkdir(const char *pathname){
    const char *curname;
    struct vnode *dir_node;
    struct vnode *newdir_node;
    struct cpiofs_internal *internal, *dirint;

    curname = pathname;
    dir_node = get_dir_vnode(&cpio_root_node, &curname);

    if(!dir_node)
        return;
    if(!curname)
        return;
    dirint = dir_node->internal;
    if(dirint->type != CPIOFS_TYPE_DIR)
        return;
    internal = kmalloc(sizeof(struct cpiofs_internal));
    newdir_node = kmalloc(sizeof(struct vnode));

    internal->name = curname;
    internal->type = CPIOFS_TYPE_DIR;
    INIT_LIST_HEAD(&internal->dir.list);
    internal->node = newdir_node;
    list_add_tail(&internal->list, &dirint->dir.list);

    newdir_node->mount = NULL;
    newdir_node->v_ops = &cpiofs_v_ops;
    newdir_node->f_ops = &cpiofs_f_ops;
    newdir_node->parent = dir_node;
    newdir_node->internal = internal;
    return;
}

static void cpio_init_create(const char *pathname, const char *data, uint64 size){
    const char *curname;
    struct vnode *dir_node;
    struct vnode *newdir_node;
    struct cpiofs_internal *internal, *dirint;

    curname = pathname;
    dir_node = get_dir_vnode(&cpio_root_node, &curname);

    if(!dir_node)
        return;
    if(!curname)
        return;
    dirint = dir_node->internal;
    if(dirint->type != CPIOFS_TYPE_DIR)
        return;
    internal = kmalloc(sizeof(struct cpiofs_internal));
    newdir_node = kmalloc(sizeof(struct vnode));

    internal->name = curname;
    internal->type = CPIOFS_TYPE_FILE;
    internal->file.data = data;
    internal->file.size = size;
    internal->node = newdir_node;
    list_add_tail(&internal->list, &dirint->dir.list);

    newdir_node->mount = NULL;
    newdir_node->v_ops = &cpiofs_v_ops;
    newdir_node->f_ops = &cpiofs_f_ops;
    newdir_node->parent = dir_node;
    newdir_node->internal = internal;
    return;
}

struct filesystem *cpiofs_init(void){
    char *cur;
    struct cpiofs_internal *internal = kmalloc(sizeof(struct cpiofs_internal));
    internal->name = NULL;
    internal->type = CPIOFS_TYPE_DIR;
    INIT_LIST_HEAD(&internal->dir.list);
    internal->node = &cpio_root_node;
    INIT_LIST_HEAD(&internal->list);

    cpio_root_node.mount = NULL;
    cpio_root_node.v_ops = &cpiofs_v_ops;
    cpio_root_node.f_ops = &cpiofs_f_ops;
    cpio_root_node.parent = NULL;
    cpio_root_node.internal = internal;

    cur = _initramfs_addr;
    while(1){
        char *component_name, *content;

        struct cpio_newc_header *pheader = (struct cpio_newc_header *)cur;
        cur += sizeof(struct cpio_newc_header);
        if(strncmp(pheader->c_magic, "070701", 6)){
            panic("[*] Only support new ASCII format for cpio. \r\n");
        }
        
        uint32 namesize = cpio_read_hex(pheader->c_namesize);
        uint32 filesize = cpio_read_hex(pheader->c_filesize);
        uint32 type     = cpio_read_hex(pheader->c_mode) & CPIO_TYPE_MASK;

        uint32 adj_namesize = ALIGN(namesize + sizeof(struct cpio_newc_header), 4) -sizeof(struct cpio_newc_header);
        uint32 adj_filesize = ALIGN(filesize, 4);
        
        component_name = cur;
        cur += adj_namesize;
        content = cur;
        cur += adj_filesize;

        if(type == CPIO_TYPE_DIR)
            cpio_init_mkdir(component_name);
        else if(type == CPIO_TYPE_FILE)
            cpio_init_create(component_name, content, filesize);

        if(namesize == 0xb && !strcmp(component_name, "TRAILER!!!"))
            break;
    }
    return &cpiofs;
}

int cpiofs_mount(struct filesystem *fs, struct mount *mount){
    struct vnode *oldnode;
    struct cpiofs_internal *internal;
    const char *name;
    preempt_disable();

    if(cpio_mounted){
        preempt_enable();
        return -1;
    }

    cpio_mounted = 1;
    preempt_enable();

    oldnode = mount->root;
    oldnode->v_ops->getname(oldnode, &name);
    internal = cpio_root_node.internal;
    internal->name = name;

    mount_old_node.mount = oldnode->mount;
    mount_old_node.v_ops = oldnode->v_ops;
    mount_old_node.f_ops = oldnode->f_ops;
    mount_old_node.parent = oldnode->parent;
    mount_old_node.internal = oldnode->internal;

    oldnode->mount = mount;
    oldnode->v_ops = cpio_root_node.v_ops;
    oldnode->f_ops = cpio_root_node.f_ops;
    oldnode->internal = internal;

    return 0;
}
int cpiofs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name){
    struct cpiofs_internal *internal, *entry;
    internal = dir_node->internal;

    if (internal->type != CPIOFS_TYPE_DIR) {
        return -1;
    }

    list_for_each_entry(entry, &internal->dir.list, list) {
        if (!strcmp(entry->name, component_name)) {
            break;
        }
    }

    if (&entry->list == &internal->dir.list) {
        return -1;
    }

    *target = entry->node;
    return 0;
}

int cpiofs_create(struct vnode *dir_node, struct vnode **target, const char *component_name){
    // TODO
    return -1;
}
int cpiofs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name){
    // TODO
    return -1;
}
int cpiofs_isdir(struct vnode *dir_node){
    struct cpiofs_internal *internal = dir_node->internal;
    if(internal->type != CPIOFS_TYPE_DIR)
        return 0;
    
    return 1;
}
int cpiofs_getname(struct vnode *dir_node, const char **name){
    struct cpiofs_internal *internal = dir_node->internal;
    *name = internal->name;
    return 0;
}
int cpiofs_getsize(struct vnode *dir_node){
    struct cpiofs_internal *internal = dir_node->internal;
    if(internal->type != CPIOFS_TYPE_FILE)
        return -1;

    return internal->file.size;
}
int cpiofs_write(struct file *file, const void *buf, size_t len){
    // TODO 
    return -1;
}
int cpiofs_read(struct file *file, void *buf, size_t len){
    struct cpiofs_internal *internal = file->vnode->internal;
    if(internal->type != CPIOFS_TYPE_FILE)
        return -1;
    
    if(len > internal->file.size - file->f_pos)
        len = internal->file.size - file->f_pos;
    
    if(!len)
        return 0;
    memncpy(buf, &internal->file.data[file->f_pos], len);
    file->f_pos += len;
    return len;
}
int cpiofs_open(struct vnode *file_node, struct file *target){
    target->vnode = file_node;
    target->f_pos = 0;
    target->f_ops = file_node->f_ops;

    return 0;
}
int cpiofs_close(struct file *file){
    file->vnode = NULL;
    file->f_pos = 0;
    file->f_ops = NULL;

    return 0;
}
long cpiofs_lseek64(struct file *file, long offset, int whence){
    int filesize;
    int base;

    filesize = file->vnode->v_ops->getsize(file->vnode);
    if(filesize < 0)
        return -1;

    switch(whence){
        case SEEK_SET:
            base = 0;
            break;
        case SEEK_CUR:
            base = file->f_pos;
            break;
        case SEEK_END:
            base = filesize;
            break;
        default:
            return -1;
    }

    if(base + offset > filesize)
        return -1;
    
    file->f_pos = base + offset;
    return 0;
}

int cpiofs_ioctl(struct file *file, uint64 request, va_list args){
    return -1;
}