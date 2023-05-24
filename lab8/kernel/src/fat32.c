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
    //const char *name;
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

    //data->name = name;
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

    list_add(&data->list, &info->dir->list);

    return node;
}

static unsigned int _get_next_cluster(unsigned int fat_lba, unsigned int cluster_id)
{
    struct cluster_entry_t *ce;
    unsigned int cid;
    unsigned char buf[BLOCK_SIZE];

    if (cluster_id >= 0x0ffffff8) {
        return cluster_id;
    }

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

        if (cid >= 0x0ffffff8) {
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
    struct vnode *node;
    struct fat_internal *internal;
    int ret;

    internal = dir_node->internal;

    if (internal->type != FAT_DIR) {
        return -1;
    }

    ret = fat32fs_lookup(dir_node, target, component_name);

    if (!ret) {
        return -1;
    }

    node = _create_vnode(dir_node, component_name, FAT_FILE, -1, 0);

    *target = node;

    return 0;
}

static int fat32fs_mkdir(struct vnode *dir_node, struct vnode **target,
                         const char *component_name)
{
    struct vnode *node;
    struct fat_internal *internal;
    int ret;

    internal = dir_node->internal;

    if (internal->type != FAT_DIR) {
        return -1;
    }

    ret = fat32fs_lookup(dir_node, target, component_name);

    if (!ret) {
        return -1;
    }

    node = _create_vnode(dir_node, component_name, FAT_DIR, -1, 0);

    *target = node;

    return 0;
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

static int _writefile_seek_cache(struct fat_internal *data, unsigned int foid,
                                 struct fat_file_block_t **block)
{
    struct fat_file_block_t *entry;
    struct list_head *head;

    head = &data->file->list;

    if (list_empty(head)) {
        return -1;
    }

    list_for_each_entry(entry, head, list) {
        *block = entry;

        if (foid == entry->oid) {
            return 0;
        }
    }

    return -1;
}

static int _writefile_seek_fat32(struct fat_internal *data,
                                 unsigned int foid, unsigned int fcid,
                                 struct fat_file_block_t **block)
{
    struct fat_info_t *info;
    unsigned int curoid, curcid;

    info = data->fat;

    if (*block) {
        curoid = (*block)->oid;
        curcid = (*block)->cid;

        if (curoid == foid) {
            return 0;
        }

        curoid++;
        curcid = _get_next_cluster(info->fat_lba, curcid);
    } else {
        curoid = 0;
        curcid = fcid;
    }

    while (1) {
        struct fat_file_block_t *newblock;

        newblock = kmalloc(sizeof(struct fat_file_block_t));

        newblock->oid = curoid;
        newblock->cid = curcid;
        newblock->bufIsUpdated = 0;

        list_add_tail(&newblock->list, &data->file->list);

        *block = newblock;

        if (curoid == foid) {
            return 0;
        }

        curoid++;
        curcid = _get_next_cluster(info->fat_lba, curcid);
    }
}

static int _writefile_cache(struct fat_internal *data, unsigned long long bckoff,
                            const unsigned char *buf, unsigned long long bufoff, unsigned int size,
                            struct fat_file_block_t *block)
{
    int wsize;

    if (!block->bufIsUpdated) {
        // read the data from sdcard
        struct fat_info_t *info;
        int lba;

        info = data->fat;
        lba = info->cluster_lba +
              (block->cid - 2) * info->bs.sector_per_cluster;

        readblock(lba, block->buf);

        block->bufIsUpdated = 1;
    }

    wsize = size > BLOCK_SIZE - bckoff ? BLOCK_SIZE - bckoff : size;

    memcpy((void *)&block->buf[bckoff], (void *)&buf[bufoff], wsize);

    return wsize;
}

static int _writefile_fat32(struct fat_internal *data, unsigned long long bckoff,
                            const unsigned char *buf, unsigned int bufoff, unsigned int size,
                            unsigned int oid, unsigned int cid)
{
    struct list_head *head;
    struct fat_info_t *info;
    struct fat_file_block_t *block;
    int lba;
    int wsize;

    head = &data->file->list;
    info = data->fat;

    block = kmalloc(sizeof(struct fat_file_block_t));

    wsize = size > BLOCK_SIZE - bckoff ? BLOCK_SIZE - bckoff : size;

    if (cid >= 0x0ffffff8) {
        memset(block->buf, 0, BLOCK_SIZE);
    } else {
        lba = info->cluster_lba + (cid - 2) * info->bs.sector_per_cluster;

        readblock(lba, block->buf);
    }

    memcpy((void *)&block->buf[bckoff], (void *)&buf[bufoff], wsize);

    block->oid = oid;
    block->cid = cid;
    block->bufIsUpdated = 1;

    list_add_tail(&block->list, head);

    return wsize;
}

static int _writefile(const void *buf, struct fat_internal *data,
                      unsigned long long fileoff, unsigned long long len)
{
    struct fat_file_block_t *block;
    struct list_head *head;
    unsigned int foid; // first block id
    unsigned int coid; // current block id
    unsigned int cid;  // target cluster id
    unsigned long long bufoff, result;
    int ret;

    block = NULL;
    head = &data->file->list;
    foid = fileoff / BLOCK_SIZE;
    coid = 0;
    cid = data->cid;
    bufoff = 0;
    result = 0;

    // Seek

    ret = _writefile_seek_cache(data, foid, &block);

    if (ret < 0) {
        ret = _writefile_seek_fat32(data, foid, cid, &block);
    }

    if (ret < 0) {
        return 0;
    }

    // if (block->oid != foid) error!

    // Write

    while (len) {
        unsigned long long bckoff;

        bckoff = (fileoff + result) % BLOCK_SIZE;

        if (&block->list != head) {
            ret = _writefile_cache(data, bckoff, buf, bufoff, len, block);

            cid = block->cid;
            block = list_first_entry(&block->list,
                                     struct fat_file_block_t, list);
        } else {
            // Read block from sdcard, create cache, then write it
            cid = _get_next_cluster(data->fat->fat_lba, cid);

            ret = _writefile_fat32(data, bckoff, buf, bufoff, len, coid, cid);
        }

        if (ret < 0) {
            break;
        }

        bufoff += ret;
        result += ret;
        coid += 1;
        len -= ret;
    }

    return result;
}

static int fat32fs_write(struct file *file, const void *buf, size_t len)
{
    struct fat_internal *data;
    int filesize;
    int ret;

    if (fat32fs_isdir(file->vnode)) {
        return -1;
    }

    if (!len) {
        return len;
    }

    filesize = fat32fs_getsize(file->vnode);

    data = file->vnode->internal;

    ret = _writefile(buf, data, file->f_pos, len);

    if (ret <= 0) {
        return ret;
    }

    file->f_pos += ret;

    if (file->f_pos > filesize) {
        data->file->size = file->f_pos;
    }

    return ret;
}

static int _readfile_seek_cache(struct fat_internal *data, unsigned int foid,
                                struct fat_file_block_t **block)
{
    struct fat_file_block_t *entry;
    struct list_head *head;

    head = &data->file->list;

    if (list_empty(head)) {
        return -1;
    }

    list_for_each_entry(entry, head, list) {
        *block = entry;

        if (foid == entry->oid) {
            return 0;
        }
    }

    return -1;
}

static int _readfile_seek_fat32(struct fat_internal *data,
                                unsigned int foid, unsigned int fcid,
                                struct fat_file_block_t **block)
{
    struct fat_info_t *info;
    unsigned int curoid, curcid;

    info = data->fat;

    if (*block) {
        curoid = (*block)->oid;
        curcid = (*block)->cid;

        if (curoid == foid) {
            return 0;
        }

        curoid++;
        curcid = _get_next_cluster(info->fat_lba, curcid);

        if (curcid >= 0x0ffffff8) {
            return -1;
        }
    } else {
        curoid = 0;
        curcid = fcid;
    }

    while (1) {
        struct fat_file_block_t *newblock;

        newblock = kmalloc(sizeof(struct fat_file_block_t));

        newblock->oid = curoid;
        newblock->cid = curcid;
        newblock->bufIsUpdated = 0;

        list_add_tail(&newblock->list, &data->file->list);

        *block = newblock;

        if (curoid == foid) {
            return 0;
        }

        curoid++;
        curcid = _get_next_cluster(info->fat_lba, curcid);

        if (curcid >= 0x0ffffff8) {
            return -1;
        }
    }
}

static int _readfile_cache(struct fat_internal *data, unsigned long long bckoff,
                           unsigned char *buf, unsigned long long bufoff, unsigned int size,
                           struct fat_file_block_t *block)
{
    int rsize;

    if (!block->bufIsUpdated) {
        // read the data from sdcard
        struct fat_info_t *info;
        int lba;

        info = data->fat;
        lba = info->cluster_lba +
              (block->cid - 2) * info->bs.sector_per_cluster;

        readblock(lba, block->buf);

        block->bufIsUpdated = 1;
    }

    rsize = size > BLOCK_SIZE - bckoff ? BLOCK_SIZE - bckoff : size;

    memcpy((void *)&buf[bufoff], (void *)&block->buf[bckoff], rsize);

    return rsize;
}

static int _readfile_fat32(struct fat_internal *data, unsigned long long bckoff,
                           unsigned char *buf, unsigned int bufoff, unsigned int size,
                           unsigned int oid, unsigned int cid)
{
    struct list_head *head;
    struct fat_info_t *info;
    struct fat_file_block_t *block;
    int lba;
    int rsize;

    head = &data->file->list;
    info = data->fat;

    block = kmalloc(sizeof(struct fat_file_block_t));

    rsize = size > BLOCK_SIZE - bckoff ? BLOCK_SIZE - bckoff : size;
    lba = info->cluster_lba + (cid - 2) * info->bs.sector_per_cluster;

    readblock(lba, block->buf);

    memcpy((void *)&buf[bufoff], (void *)&block->buf[bckoff], rsize);

    block->oid = oid;
    block->cid = cid;
    block->bufIsUpdated = 1;

    list_add_tail(&block->list, head);

    return rsize;
}

static int _readfile(void *buf, struct fat_internal *data,
                     unsigned long long fileoff, unsigned long long len)
{
    struct fat_file_block_t *block;
    struct list_head *head;
    unsigned int foid; // first block id
    unsigned int coid; // current block id
    unsigned int cid;  // target cluster id
    unsigned long long bufoff, result;
    int ret;

    block = NULL;
    head = &data->file->list;
    foid = fileoff / BLOCK_SIZE;
    coid = 0;
    cid = data->cid;
    bufoff = 0;
    result = 0;

    // Seek

    ret = _readfile_seek_cache(data, foid, &block);

    if (ret < 0) {
        ret = _readfile_seek_fat32(data, foid, cid, &block);
    }

    if (ret < 0) {
        return 0;
    }

    // if (block->oid != foid) error!

    // Read

    while (len) {
        unsigned long long bckoff;

        bckoff = (fileoff + result) % BLOCK_SIZE;

        if (&block->list != head) {
            ret = _readfile_cache(data, bckoff, buf, bufoff, len, block);

            cid = block->cid;
            block = list_first_entry(&block->list,
                                     struct fat_file_block_t, list);
        } else {
            // Read block from sdcard, create cache
            cid = _get_next_cluster(data->fat->fat_lba, cid);

            if (cid >= 0x0ffffff8) {
                break;
            }

            ret = _readfile_fat32(data, bckoff, buf, bufoff, len, coid, cid);
        }

        if (ret < 0) {
            break;
        }

        bufoff += ret;
        result += ret;
        coid += 1;
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


