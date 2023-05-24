#include "fat32.h"
#include "memory.h"
#include "sdhost.h"
#include "string.h"
#include "vfs.h"
#include "list.h"
#include <stdarg.h>
#include "uart1.h"

static int fat32fs_mount(struct filesystem *fs, struct mount *mount);

static int fat32fs_lookup(struct vnode *dir_node, struct vnode **target, const char *component_name);
static int fat32fs_create(struct vnode *dir_node, struct vnode **target, const char *component_name);
static int fat32fs_mkdir(struct vnode *dir_node, struct vnode **target, const char *component_name);
static int fat32fs_isdir(struct vnode *dir_node);
static int fat32fs_getname(struct vnode *dir_node, const char **name);
static int fat32fs_getsize(struct vnode *dir_node);

static struct vnode_operations fat32fs_v_ops = {
    .lookup = fat32fs_lookup,
    .create = fat32fs_create,
    .mkdir = fat32fs_mkdir,
    .isdir = fat32fs_isdir,
    .getname = fat32fs_getname,
    .getsize = fat32fs_getsize
};

static int fat32fs_write(struct file *file, const void *buf, size_t len);
static int fat32fs_read(struct file *file, void *buf, size_t len);
static int fat32fs_open(struct vnode *file_node, struct file **target);
static int fat32fs_close(struct file *file);
static long fat32fs_lseek64(struct file *file, long offset, int whence);

static struct file_operations fat32fs_f_ops = {
    .write = fat32fs_write,
    .read = fat32fs_read,
    .open = fat32fs_open,
    .close = fat32fs_close,
    .lseek64 = fat32fs_lseek64,
};

/* filesystem methods */

static int fat32fs_mount(struct filesystem *fs, struct mount *mount)
{
    struct partition_t *partition;
    struct fat_info_t *fat;
    struct fat_dir_t *dir;
    struct fat_internal *data;
    struct vnode *oldnode, *node;
    const char *name;
    unsigned int lba;
    unsigned char buf[BLOCK_SIZE];

    readblock(0, buf);

    partition = (struct partition_t *)&buf[0x1be];

    if (buf[510] != 0x55 || buf[511] != 0xaa) {
        return -1;
    }

    if (partition[0].type != 0xb && partition[0].type != 0xc) {
        return -1;
    }

    lba = partition[0].lba;

    readblock(partition[0].lba, buf);

    node = kmalloc(sizeof(struct vnode));
    data = kmalloc(sizeof(struct fat_internal));
    fat = kmalloc(sizeof(struct fat_info_t));
    dir = kmalloc(sizeof(struct fat_dir_t));

    memcpy((void *)&fat->bs, (void *)buf, sizeof(fat->bs));

    fat->fat_lba = lba + fat->bs.reserved_sector_cnt;
    fat->cluster_lba = fat->fat_lba +
                       fat->bs.fat_cnt * fat->bs.sector_per_fat32;

    INIT_LIST_HEAD(&dir->list);

    oldnode = mount->root;
    //oldnode->v_ops->getname(oldnode, &name);

    node->mount = 0;
    node->v_ops = oldnode->v_ops;
    node->f_ops = oldnode->f_ops;
    node->parent = oldnode->parent;
    node->internal = oldnode->internal;

    data->name = name;
    data->node = node;
    data->fat = fat;
    data->cid = 2;
    data->type = FAT_DIR;
    data->dir = dir;

    oldnode->mount = mount;
    oldnode->v_ops = &fat32fs_v_ops;
    oldnode->f_ops = &fat32fs_f_ops;
    oldnode->internal = data;

    return 0;
}

/* vnode_operations methods */

static struct vnode *_create_vnode(struct vnode *parent,
                                   const char *name,
                                   unsigned int type,
                                   unsigned int cid,
                                   unsigned int size)
{
    struct vnode *node;
    struct fat_internal *info, *data;
    char *buf;
    int len;

    info = parent->internal;
    len = strlen(name);

    buf = kmalloc(len + 1);
    node = kmalloc(sizeof(struct vnode));
    data = kmalloc(sizeof(struct fat_internal));

    strcpy(buf, name);

    data->name = buf;
    data->node = node;
    data->fat = info->fat;
    data->cid = cid;
    data->type = type;

    if (type == FAT_DIR) {
        struct fat_dir_t *dir;

        dir = kmalloc(sizeof(struct fat_dir_t));

        INIT_LIST_HEAD(&dir->list);

        data->dir = dir;
    } else {
        struct fat_file_t *file;

        file = kmalloc(sizeof(struct fat_file_t));

        INIT_LIST_HEAD(&file->list);
        file->size = size;

        data->file = file;
    }

    node->mount = parent->mount;
    node->v_ops = &fat32fs_v_ops;
    node->f_ops = &fat32fs_f_ops;
    node->parent = parent;
    node->internal = data;

    return node;
}

static unsigned int _get_next_cluster(unsigned int fat_lba, unsigned int cluster_id)
{
    struct cluster_entry_t *ce;
    unsigned int cid;
    unsigned char buf[BLOCK_SIZE];

    fat_lba += cluster_id / CLUSTER_ENTRY_PER_BLOCK;
    cid = cluster_id % CLUSTER_ENTRY_PER_BLOCK;

    // TODO: Cache FAT
    readblock(fat_lba, buf);

    ce = &(((struct cluster_entry_t *)buf)[cid]);

    return ce->val;
}

static int _lookup_cache(struct vnode *dir_node, struct vnode **target,
                         const char *component_name)
{
    struct fat_internal *data, *entry;
    struct fat_dir_t *dir;
    int found;

    data = dir_node->internal;
    dir = data->dir;

    found = 0;

    list_for_each_entry(entry, &dir->list, list) {
        if (!strcasecmp(component_name, entry->name)) {
            found = 1;
            break;
        }
    }

    if (!found) {
        return -1;
    }

    *target = entry->node;

    return 0;
}

static struct dir_t *__lookup_fat32(struct vnode *dir_node,
                                    const char *component_name,
                                    unsigned char *buf)
{
    struct dir_t *dir;
    struct fat_internal *data;
    struct fat_info_t *fat;
    struct filename_t name;
    unsigned int cid;
    int found, dirend, lfn;

    data = dir_node->internal;
    fat = data->fat;
    cid = data->cid;

    found = 0;
    dirend = 0;

    memset(&name, 0, sizeof(struct filename_t));

    while (1) {
        int lba;

        lba = fat->cluster_lba + (cid - 2) * fat->bs.sector_per_cluster;

        // TODO: Cache data block of directory
        readblock(lba, buf);

        for (int i = 0; i < DIR_PER_BLOCK; ++i) {
            unsigned char len;

            dir = (struct dir_t *)(&buf[sizeof(struct dir_t) * i]);

            if (dir->name[0] == 0) {
                dirend = 1;
                break;
            }

            if ((dir->attr & 0xf) == 0xf) {
                struct long_dir_t *ldir;
                int n;

                lfn = 1;

                ldir = (struct long_dir_t *)dir;
                n = (dir->name[0] & 0x3f) - 1;

                for (int i = 0; ldir->name1[i] != 0xff && i < 10; i += 2) {
                    name.part[n].name[i / 2] = ldir->name1[i];
                }
                for (int i = 0; ldir->name2[i] != 0xff && i < 12; i += 2) {
                    name.part[n].name[5 + i / 2] = ldir->name2[i];
                }
                for (int i = 0; ldir->name3[i] != 0xff && i < 4; i += 2) {
                    name.part[n].name[11 + i / 2] = ldir->name3[i];
                }

                continue;
            }

            if (lfn == 1) {
                if (!strcasecmp(component_name, (void *)name.fullname)) {
                    found = 1;
                    break;
                }

                lfn = 0;
                memset(&name, 0, sizeof(struct filename_t));

                continue;
            }

            lfn = 0;
            len = 8;

            while (len) {
                if (dir->name[len - 1] != 0x20) {
                    break;
                }
                len -= 1;
            }

            memcpy((void *)name.fullname, (void *)dir->name, len);
            name.fullname[len] = 0;

            len = 3;

            while (len) {
                if (dir->name[8 + len - 1] != 0x20) {
                    break;
                }
                len -= 1;
            }

            if (len >= 0) {
                strcat((void *)name.fullname, ".");
                strncat((void *)name.fullname, (void *)&dir->name[8], len);
            }

            if (!strcasecmp(component_name, (void *)name.fullname)) {
                found = 1;
                break;
            }

            memset(&name, 0, sizeof(struct filename_t));
        }

        if (found) {
            break;
        }

        if (dirend) {
            break;
        }

        cid = _get_next_cluster(fat->fat_lba, cid);

        if (cid > 0x0ffffff8) {
            break;
        }
    }

    if (!found) {
        return NULL;
    }

    return dir;
}

static int _lookup_fat32(struct vnode *dir_node, struct vnode **target,
                         const char *component_name)
{
    struct vnode *node;
    struct dir_t *dir;
    struct fat_internal *data, *nodedata;
    unsigned int type, cid;
    unsigned char buf[BLOCK_SIZE];

    dir = __lookup_fat32(dir_node, component_name, buf);

    if (!dir) {
        return -1;
    }

    if (!(dir->attr & ATTR_FILE_DIR_MASK)) {
        return -1;
    }

    cid = (dir->ch << 16) | dir->cl;

    if (dir->attr & ATTR_ARCHIVE) {
        type = FAT_FILE;
    } else {
        type = FAT_DIR;
    }

    node = _create_vnode(dir_node, component_name, type, cid, dir->size);

    data = dir_node->internal;
    nodedata = node->internal;

    list_add(&nodedata->list, &data->dir->list);

    *target = node;

    return 0;
}

static int fat32fs_lookup(struct vnode *dir_node, struct vnode **target,
                          const char *component_name)
{
    struct fat_internal *data;
    int ret;

    data = dir_node->internal;

    if (data->type != FAT_DIR) {
        return -1;
    }

    ret = _lookup_cache(dir_node, target, component_name);

    if (ret >= 0) {
        return ret;
    }

    return _lookup_fat32(dir_node, target, component_name);
}

static int fat32fs_create(struct vnode *dir_node, struct vnode **target,
                          const char *component_name)
{
    // TODO
    return -1;
}

static int fat32fs_mkdir(struct vnode *dir_node, struct vnode **target,
                         const char *component_name)
{
    // TODO
    return -1;
}

static int fat32fs_isdir(struct vnode *dir_node)
{
    struct fat_internal *internal;

    internal = dir_node->internal;

    if (internal->type != FAT_DIR) {
        return 0;
    }

    return 1;
}

static int fat32fs_getname(struct vnode *dir_node, const char **name)
{
    struct fat_internal *internal;

    internal = dir_node->internal;

    *name = internal->name;

    return 0;
}

static int fat32fs_getsize(struct vnode *dir_node)
{
    struct fat_internal *internal;

    internal = dir_node->internal;

    if (internal->type == FAT_DIR) {
        return 0;
    }

    return internal->file->size;
}

/* file_operations methods */

static int fat32fs_write(struct file *file, const void *buf, size_t len)
{
    // TODO
    return -1;
}

static int _readfile_cache(struct fat_internal *data, unsigned long long bckoff,
                           unsigned char *buf, unsigned long long bufoff,
                           unsigned int oid, unsigned int size,
                           struct list_head **iter,
                           unsigned int *cid)
{
    struct list_head *tmpiter;
    struct fat_file_t *file;
    struct fat_file_block_t *entry;
    int rsize;

    tmpiter = *iter;
    file = data->file;

    iter_for_each_entry(entry, tmpiter, &file->list, list) {
        if (oid <= entry->oid) {
            break;
        }
    }

    if (&entry->list == &file->list) {
        *iter = &entry->list;
        return -1;
    }

    if (oid < entry->oid) {
        *iter = &entry->list;
        return -2;
    }

    rsize = size > BLOCK_SIZE - bckoff ? BLOCK_SIZE - bckoff : size;

    memcpy((void *)&buf[bufoff], (void *)&entry->buf[bckoff], rsize);

    *cid = entry->cid;

    *iter = &entry->list;
    return rsize;
}

static int _readfile_fat32(struct fat_internal *data, unsigned long long bckoff,
                           unsigned char *buf, unsigned int bufoff,
                           unsigned int oid, unsigned int size,
                           struct list_head *list,
                           unsigned int cid)
{
    struct fat_info_t *info;
    struct fat_file_block_t *block;
    int lba;
    int rsize;

    info = data->fat;

    block = kmalloc(sizeof(struct fat_file_block_t));

    rsize = size > BLOCK_SIZE - bckoff ? BLOCK_SIZE - bckoff : size;
    lba = info->cluster_lba + (cid - 2) * info->bs.sector_per_cluster;

    readblock(lba, block->buf);

    memcpy((void *)&buf[bufoff], (void *)&block->buf[bckoff], rsize);

    block->oid = oid;
    block->cid = cid;

    list_add_tail(&block->list, list);

    return rsize;
}

static int _readfile(void *buf, struct fat_internal *data,
                     unsigned long long fileoff, unsigned long long len)
{
    struct list_head *iter;
    unsigned int oid, cid;
    unsigned long long bufoff, result;
    int cache_end;

    iter = &data->file->list;
    oid = fileoff / BLOCK_SIZE;
    cid = 0;
    bufoff = 0;
    result = 0;
    cache_end = list_empty(iter);

    if (!cache_end) {
        iter = iter->next;
    }

    while (len) {
        unsigned long long bckoff;
        int cache_hit, ret;

        bckoff = (fileoff + result) % BLOCK_SIZE;
        cache_hit = 0;

        if (!cache_end) {
            // Search cache
            ret = _readfile_cache(data, bckoff, buf, bufoff,
                                  oid, len, &iter, &cid);

            if (ret == -1) {
                cache_end = 1;
            } else if (ret > 0) {
                cache_hit = 1;
            }
        }

        if (!cache_hit) {
            // Read block from sdcard, create cache
            if (!cid) {
                cid = data->cid;
            } else {
                cid = _get_next_cluster(data->fat->fat_lba, cid);
            }

            if (cid > 0x0ffffff8) {
                break;
            }

            ret = _readfile_fat32(data, bckoff, buf, bufoff,
                                  oid, len, iter, cid);
        }

        if (ret < 0) {
            break;
        }

        bufoff += ret;
        result += ret;
        oid += 1;
        len -= ret;
    }

    return result;
}

static int fat32fs_read(struct file *file, void *buf, size_t len)
{
    struct fat_internal *data;
    int filesize;
    int ret;

    if (fat32fs_isdir(file->vnode)) {
        return -1;
    }

    filesize = fat32fs_getsize(file->vnode);
    data = file->vnode->internal;

    if (file->f_pos + len > filesize) {
        len = filesize - file->f_pos;
    }
    if (!len) {
        return len;
    }

    ret = _readfile(buf, data, file->f_pos, len);
    if (ret <= 0) {
        return ret;
    }

    file->f_pos += ret;
    return ret;
}

static int fat32fs_open(struct vnode *file_node, struct file **target)
{
    (*target)->vnode = file_node;
    (*target)->f_pos = 0;
    (*target)->f_ops = file_node->f_ops;
    return 0;
}

static int fat32fs_close(struct file *file)
{
    file->vnode = NULL;
    file->f_pos = 0;
    file->f_ops = NULL;

    return 0;
}

static long fat32fs_lseek64(struct file *file, long offset, int whence)
{
    int filesize;
    int base;

    if (!fat32fs_isdir(file->vnode)) {
        return -1;
    }

    filesize = fat32fs_getsize(file->vnode);

    if (filesize < 0) {
        return -1;
    }

    switch (whence) {
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

    if (base + offset > filesize) {
        return -1;
    }

    file->f_pos = base + offset;

    return 0;
}

int register_fat32()
{
    struct filesystem fs;
    fs.name = "fat32";
    fs.setup_mount = fat32fs_mount;
    return register_filesystem(&fs);
}


